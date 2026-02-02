#include "detail/executor.hpp"
#include "detail/executors/platform_executor.hpp"
#include "detail/executors/lua_executor.hpp"

#include <spdlog/spdlog.h>

namespace xxlib {
	namespace executor {
		std::expected<int32_t, std::string> execute_command(Command& command, CommandContext& context) {
			if (command.executionEngine == CommandExecutionEngine::System) {
				return xxlib::platform_executor::execute_command(command, context);
			} else if (command.executionEngine == CommandExecutionEngine::Lua) {
				return xxlib::lua_executor::execute_command(command, context);
			} else {
				return std::unexpected("Unknown execution engine");
			}
		}
	} // namespace executor
} // namespace xxlib
