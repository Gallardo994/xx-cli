#include "detail/executors/lua_executor.hpp"
#include "detail/executors/platform_executor.hpp"
#include "detail/luavm.hpp"
#include "detail/command.hpp"
#include "detail/helpers.hpp"

#include <expected>
#include <spdlog/spdlog.h>

namespace xxlib {
	namespace lua_executor {
		std::expected<int32_t, std::string> execute_command(const Command& command, bool dryRun) {
			auto state = xxlib::luavm::create();
			std::string luaCommand;

			for (const auto& part : command.cmd) {
				luaCommand += xxlib::command::render(part, command.templateVars, command.renderEngine) + " ";
			}

			if (dryRun) {
				spdlog::info("Lua script to be executed: {}", luaCommand);
				return 0;
			}

			if (command.requiresConfirmation) {
				if (xxlib::helpers::ask_for_confirmation("Lua Executor Engine wants to run: " + luaCommand)) {
					spdlog::info("User confirmed execution.");
				} else {
					spdlog::info("User denied execution.");
					return 0;
				}
			}

			spdlog::debug("Executing Lua script: {}", luaCommand);

			const auto push_as_table = [&](const std::unordered_map<std::string, std::string>& map, const char* tableName) {
				xxlib::luavm::new_table(state);
				for (const auto& [key, value] : map) {
					xxlib::luavm::push_string(state, key);
					xxlib::luavm::push_string(state, value);
					xxlib::luavm::set_table(state, -3);
				}
				xxlib::luavm::set_global(state, tableName);
			};

			push_as_table(command.templateVars, "TEMPLATE_VARS");
			push_as_table(command.envs, "ENVS");

			xxlib::luavm::new_table(state);
			{
				xxlib::luavm::push_string(state, "command_name");
				xxlib::luavm::push_string(state, command.name);
				xxlib::luavm::set_table(state, -3);
			}
			xxlib::luavm::set_global(state, "CTX");

			auto loadStatus = xxlib::luavm::loadstring(state, luaCommand);
			if (loadStatus != 0) {
				return std::unexpected(std::string("Error executing Lua command: ") + xxlib::luavm::tostring(state));
			}

			auto pcallStatus = xxlib::luavm::pcall(state, 0, 1, 0);
			if (pcallStatus != 0) {
				return std::unexpected(std::string("Error loading Lua command: ") + xxlib::luavm::tostring(state));
			}

			if (xxlib::luavm::is_nil(state)) {
				spdlog::info("Lua script returned nil, treating as successful no-op.");
				return 0;

			} else if (xxlib::luavm::is_integer(state)) {
				auto exitCode = static_cast<int32_t>(xxlib::luavm::tointeger(state));
				spdlog::debug("Lua script returned exit code: {}", exitCode);
				return exitCode;

			} else if (xxlib::luavm::is_boolean(state)) {
				auto boolResult = xxlib::luavm::toboolean(state);
				auto exitCode = boolResult ? 0 : 1;
				spdlog::debug("Lua script returned boolean: {}, treating as exit code: {}", boolResult, exitCode);
				return exitCode;

			} else if (xxlib::luavm::is_string(state)) {
				const auto* shellCommand = xxlib::luavm::tostring(state);
				spdlog::debug("Lua script returned shell command: {}", shellCommand);

				Command shellExecCommand = command;
				shellExecCommand.cmd = {shellCommand};
				shellExecCommand.executionEngine = CommandExecutionEngine::System;
				shellExecCommand.requiresConfirmation = false;

				return xxlib::platform_executor::execute_command(shellExecCommand, false);

			} else {
				return std::unexpected("Lua script returned unsupported type");
			}
		}
	} // namespace lua_executor
} // namespace xxlib
