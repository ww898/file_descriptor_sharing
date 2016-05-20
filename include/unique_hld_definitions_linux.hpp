#pragma once

#include "unique_hld.hpp"

namespace jetbrains {
namespace common {

struct deleter_close { void operator()(int const _Hdl) const throw() { close(_Hdl); } };

using unique_hld_close = jetbrains::common::unique_hld<int, deleter_close, -1>;

}}
