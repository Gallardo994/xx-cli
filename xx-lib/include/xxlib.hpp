#ifndef XXLIB_HPP
#define XXLIB_HPP

#include <string>

namespace xxlib {
	[[nodiscard]] inline std::string version() {
		return "1.1.0";
	}
} // namespace xxlib

#include "detail/parser.hpp"
#include "detail/planner.hpp"
#include "detail/executor.hpp"
#include "detail/luavm.hpp"
#include "detail/platform.hpp"
#include "detail/command.hpp"
#include "detail/helpers.hpp"
#include "detail/updates.hpp"

#endif // XXLIB_HPP
