#ifndef XX_LUA_EXECUTOR_HPP
#define XX_LUA_EXECUTOR_HPP

#include <string>
#include <cstdint>
#include <expected>

class Command;
class CommandContext;

namespace xxlib::lua_executor {
	[[nodiscard]] std::expected<int32_t, std::string> execute_command(Command& command, CommandContext& context);
} // namespace xxlib::lua_executor

#endif // XX_LUA_EXECUTOR_HPP
