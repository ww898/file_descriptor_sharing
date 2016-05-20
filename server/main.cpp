#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include "unique_hld_definitions_linux.hpp"
#include "config.hpp"

int main()
{
    using namespace jetbrains;

    std::cout << "Server" << std::endl;
    common::unique_hld_close const _Socket_fd(socket(AF_UNIX, SOCK_STREAM, 0));
    if (!_Socket_fd)
    {
        perror("Failed to open socket");
        return 1;
    }

    sockaddr_un _Sa_un;
    memset(&_Sa_un, 0, sizeof(_Sa_un));
    _Sa_un.sun_family = AF_UNIX;
    strcpy(_Sa_un.sun_path, P_tmpdir);
    strcat(_Sa_un.sun_path, _Ourun_path);
    unlink(_Sa_un.sun_path);
    if (bind(_Socket_fd.get(), reinterpret_cast<sockaddr const *>(&_Sa_un), sizeof(_Sa_un)) < 0)
    {
        perror("Failed to bind socket");
        return 1;
    }

    if (listen(_Socket_fd.get(), 10) < 0)
    {
        perror("Failed to listen socket");
        return 1;
    }

    std::cout << "Press Ctrl+C to stop..." << std::endl;

    while (true)
    {
        timeval _Tv;
        _Tv.tv_sec = 1;
        _Tv.tv_usec = 0;

        fd_set _Rfds;
        FD_ZERO(&_Rfds);
        FD_SET(_Socket_fd.get(), &_Rfds);
        if (select(_Socket_fd.get() + 1, &_Rfds, nullptr, nullptr, &_Tv) < 0)
        {
            perror("Failed to select socket");
            return 1;
        }

        if (FD_ISSET(_Socket_fd.get(), &_Rfds))
        {
            common::unique_hld_close const _Accept_fd(accept(_Socket_fd.get(), nullptr, nullptr));
            if (!_Accept_fd)
            {
                perror("Failed to accept socket");
                return 1;
            }

            common::unique_hld_close _File_fd;

            {
                unsigned char _Iovd[1024];
                iovec _Iov;
                memset(&_Iov, 0, sizeof(_Iov));
                _Iov.iov_base = _Iovd;
                _Iov.iov_len = sizeof(_Iovd);

                unsigned char _Acm[1024];
                msghdr _Msg;
                memset(&_Msg, 0, sizeof(_Msg));
                _Msg.msg_iov = &_Iov;
                _Msg.msg_iovlen = 1;
                _Msg.msg_control = _Acm;
                _Msg.msg_controllen = sizeof(_Acm);

                if (recvmsg(_Accept_fd.get(), &_Msg, 0) < 0)
                {
                    perror("Failed to receive from socket");
                    return 1;
                }

                for (auto _Cm = CMSG_FIRSTHDR(&_Msg); _Cm; _Cm = CMSG_NXTHDR(&_Msg, _Cm))
                    if (_Cm->cmsg_type == SCM_RIGHTS)
                        _File_fd.reset(*reinterpret_cast<int *>(CMSG_DATA(&_Acm)));

                if (!_File_fd)
                {
                    perror("Failed to check file socket");
                    return 1;
                }
            }

            {
                static unsigned char const _Ourdata[] = { 0xAA, 0xBB, 0XCC };
                write(_File_fd.get(), _Ourdata, sizeof(_Ourdata));
            }

            {
                static unsigned char const _Ourdata = 0xAA;
                iovec _Iov;
                memset(&_Iov, 0, sizeof(_Iov));
                _Iov.iov_base = const_cast<unsigned char *>(&_Ourdata);
                _Iov.iov_len = sizeof(_Ourdata);

                msghdr _Msg;
                memset(&_Msg, 0, sizeof(_Msg));
                _Msg.msg_iov = &_Iov;
                _Msg.msg_iovlen = 1;

                if (sendmsg(_Accept_fd.get(), &_Msg, 0) < 0)
                {
                    perror("Failed to send to socket");
                    return 1;
                }
            }
        }
    }

    std::cout << "Done" << std::endl;
    return 0;
}
