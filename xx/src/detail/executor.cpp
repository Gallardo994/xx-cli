#include "detail/executor.hpp"
#include "detail/executors/platform_executor.hpp"
#include "detail/executors/lua_executor.hpp"

#include <spdlog/spdlog.h>

namespace xxlib {
	namespace executor {
		std::expected<int32_t, std::string> execute_command(const Command& command, bool dryRun) {
			if (command.executionEngine == CommandExecutionEngine::System) {
				return xxlib::platform_executor::execute_command(command, dryRun);
			} else if (command.executionEngine == CommandExecutionEngine::Lua) {
				return xxlib::lua_executor::execute_command(command, dryRun);
			} else {
				return std::unexpected("Unknown execution engine");
			}
		}
	} // namespace executor
} // namespace xxlib
