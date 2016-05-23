#pragma once

#include <functional>

namespace jetbrains {
namespace common {

struct scope_exit final
{
    explicit scope_exit(std::function<void()> && _Fn) :
            _Myfn(std::move(_Fn))
    {
    }

    ~scope_exit()
    {
        _Myfn();
    }

private:
    std::function<void()> const _Myfn;

    scope_exit(scope_exit const & _Right) = delete;
    scope_exit & operator=(scope_exit const & _Right) = delete;

    scope_exit(scope_exit && _Right) = delete;
    scope_exit & operator=(scope_exit && _Right) = delete;
};

}
}
