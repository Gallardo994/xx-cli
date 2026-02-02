#ifndef XXLIB_HPP
#define XXLIB_HPP

#include <string>

namespace xxlib {
    inline std::string version() {
        return "0.6.0";
    }
} // namespace xxlib

#include "detail/parser.hpp"
#include "detail/planner.hpp"
#include "detail/executor.hpp"
#include "detail/luavm.hpp"

#endif // XXLIB_HPP
