#ifndef XXLIB_HPP
#define XXLIB_HPP

#include <string>

namespace xxlib {
	[[nodiscard]] std::string version();
	[[nodiscard]] std::string detailed_version_text();
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
