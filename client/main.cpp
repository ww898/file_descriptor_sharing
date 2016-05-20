#include <memory>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <iomanip>
#include "unique_hld_definitions_linux.hpp"
#include "config.hpp"

int main()
{
    using namespace jetbrains;

    std::cout << "Client" << std::endl;
    common::unique_hld_close const _File_fd(open(P_tmpdir, O_TMPFILE | O_RDWR));
    if (!_File_fd)
    {
        perror("Failed to open temp file");
        return 1;
    }

    common::unique_hld_close const _Socket_fd(socket(AF_UNIX, SOCK_STREAM, 0));
    if (!_Socket_fd)
    {
        perror("Failed to open socket");
        return 1;
    }

    {
        sockaddr_un _Sa_un;
        memset(&_Sa_un, 0, sizeof(_Sa_un));
        _Sa_un.sun_family = AF_UNIX;
        strcpy(_Sa_un.sun_path, P_tmpdir);
        strcat(_Sa_un.sun_path, _Ourun_path);
        if (connect(_Socket_fd.get(), reinterpret_cast<sockaddr const *>(&_Sa_un), sizeof(_Sa_un)) < 0)
        {
            perror("Failed to connect to server");
            return 1;
        }
    }

    {
#pragma pack(push, 1)
        union
        {
            cmsghdr _Mycm;
            unsigned char _Mycontrol[CMSG_SPACE(sizeof(int))];
        } _Acm;
#pragma pack(pop)
        memset(&_Acm, 0, sizeof(_Acm));
        _Acm._Mycm.cmsg_len = CMSG_LEN(sizeof(int));
        _Acm._Mycm.cmsg_level = SOL_SOCKET;
        _Acm._Mycm.cmsg_type = SCM_RIGHTS;
        *reinterpret_cast<int *>(CMSG_DATA(&_Acm)) = _File_fd.get();

        static unsigned char const _Ourdata = 0x55;
        iovec _Iov;
        memset(&_Iov, 0, sizeof(_Iov));
        _Iov.iov_base = const_cast<unsigned char *>(&_Ourdata);
        _Iov.iov_len = sizeof(_Ourdata);

        msghdr _Msg;
        memset(&_Msg, 0, sizeof(_Msg));
        _Msg.msg_iov = &_Iov;
        _Msg.msg_iovlen = 1;
        _Msg.msg_control = _Acm._Mycontrol;
        _Msg.msg_controllen = sizeof(_Acm._Mycontrol);

        if (sendmsg(_Socket_fd.get(), &_Msg, 0) < 0)
        {
            perror("Failed to sent socket");
            return 1;
        }
    }

    {
        unsigned char _Buf[1024];
        iovec _Iov;
        _Iov.iov_base = _Buf;
        _Iov.iov_len = sizeof(_Buf);

        msghdr _Msg;
        memset(&_Msg, 0, sizeof(_Msg));
        _Msg.msg_iov = &_Iov;
        _Msg.msg_iovlen = 1;

        auto const _Size = recvmsg(_Socket_fd.get(), &_Msg, 0);
        if (_Size < 0)
        {
            perror("Failed to sent socket");
            return 1;
        }
    }

    if (lseek(_File_fd.get(), 0, SEEK_SET) < 0)
    {
        perror("Failed to seek");
        return 1;
    }

    unsigned char _Buf[1024];
    auto const _Size = read(_File_fd.get(), _Buf, sizeof(_Buf));
    if (_Size < 0)
    {
        perror("Failed to read from file");
        return 1;
    }

    std::cout << std::hex << std::setw(2) << std::setfill('0');
    for (int _N = 0; _N < _Size; ++_N)
        std::cout << " 0x" << static_cast<unsigned>(_Buf[_N]);
    std::cout << std::endl;
    return 0;
}