#ifndef XXLIB_HPP
#define XXLIB_HPP

#include <string>

namespace xxlib {
    [[nodiscard]] inline std::string version() {
        return "0.9.3";
    }
} // namespace xxlib

#include "detail/parser.hpp"
#include "detail/planner.hpp"
#include "detail/executor.hpp"
#include "detail/luavm.hpp"
#include "detail/platform.hpp"
#include "detail/command.hpp"

#endif // XXLIB_HPP
