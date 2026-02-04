#include "detail/executor.hpp"
#include "detail/executors/platform_executor.hpp"
#include "detail/executors/lua_executor.hpp"
#include "detail/executors/dotnet_run_executor.hpp"
#include "detail/command.hpp"

#include <stdexcept>

namespace xxlib::executor {
	Engine string_to_execution_engine(const std::string& executorStr) {
		if (executorStr == "system") {
			return Engine::System;
		} else if (executorStr == "lua") {
			return Engine::Lua;
		} else if (executorStr == "dotnet_run") {
			return Engine::DotnetRun;
		} else {
			throw std::invalid_argument("Unknown executor type: " + executorStr);
		}
	}

	std::expected<int32_t, std::string> execute_command(Command& command, CommandContext& context) {
		if (command.executionEngine == Engine::System) {
			return xxlib::platform_executor::execute_command(command, context);
		} else if (command.executionEngine == Engine::Lua) {
			return xxlib::lua_executor::execute_command(command, context);
		} else if (command.executionEngine == Engine::DotnetRun) {
			return xxlib::dotnet_run_executor::execute_command(command, context);
		} else {
			return std::unexpected("Unknown execution engine");
		}
	}
} // namespace xxlib::executor
