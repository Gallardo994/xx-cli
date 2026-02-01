#ifndef XX_LUA_EXECUTOR_HPP
#define XX_LUA_EXECUTOR_HPP

#include "detail/command.hpp"
#include <string>
#include <cstdint>
#include <expected>

namespace xxlib {
	namespace lua_executor {
		std::expected<int32_t, std::string> execute_command(const Command& command, bool dryRun);
	} // namespace lua_executor
} // namespace xxlib

#endif // XX_LUA_EXECUTOR_HPP
