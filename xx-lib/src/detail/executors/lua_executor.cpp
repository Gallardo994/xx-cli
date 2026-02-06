#include "detail/executor.hpp"
#include "detail/executors/lua_executor.hpp"
#include "detail/executors/platform_executor.hpp"
#include "detail/luavm.hpp"
#include "detail/command.hpp"
#include "detail/helpers.hpp"
#include "detail/platform.hpp"
#include "detail/renderer.hpp"

#include <expected>
#include <spdlog/spdlog.h>

namespace xxlib::lua_executor {
	void push_as_table(xxlib::luavm::LuaStatePtr& state, const std::unordered_map<std::string, std::string>& map, const std::string& tableName) {
		xxlib::luavm::new_table(state);
		for (const auto& [key, value] : map) {
			xxlib::luavm::push_string(state, key);
			xxlib::luavm::push_string(state, value);
			xxlib::luavm::set_table(state, -3);
		}
		xxlib::luavm::set_global(state, tableName);
	}

	void push_as_table(xxlib::luavm::LuaStatePtr& state, const std::vector<std::string>& vec, const std::string& tableName) {
		xxlib::luavm::new_table(state);
		auto index = 1;
		for (const auto& value : vec) {
			xxlib::luavm::push_string(state, value);
			xxlib::luavm::seti(state, -2, index++);
		}
		xxlib::luavm::set_global(state, tableName);
	}

	std::expected<int32_t, std::string> execute_command(Command& command, CommandContext& context) {
		const auto extrasResult = xxlib::helpers::split_extras(context.extras);
		std::vector<std::string> positional;

		for (const auto& [key, value] : extrasResult.kv) {
			if (command.templateVars.find(key) != command.templateVars.end()) {
				command.templateVars[key] = value;
				spdlog::debug("Setting template variable: {}={}", key, value);
				continue;
			}
		}

		const auto unsetVars = xxlib::helpers::get_uset_vars(command.templateVars);
		if (!unsetVars.empty()) {
			std::ostringstream errOss;
			errOss << "The following template variables are not set: ";
			for (size_t i = 0; i < unsetVars.size(); ++i) {
				errOss << unsetVars[i];
				if (i < unsetVars.size() - 1) {
					errOss << ", ";
				}
			}
			return std::unexpected(errOss.str());
		}

		std::string luaCommand;
		for (const auto& part : command.cmd) {
			luaCommand += xxlib::renderer::render(part, command.templateVars, command.renderEngine) + " ";
		}

		if (context.dryRun) {
			spdlog::info("Lua script to be executed: {}", luaCommand);
			return 0;
		}

		if (command.requiresConfirmation) {
			if (xxlib::helpers::ask_for_confirmation("Lua Executor Engine wants to run: \n" + luaCommand)) {
				spdlog::debug("User confirmed execution.");
			} else {
				spdlog::debug("User denied execution.");
				return 0;
			}
		}

		spdlog::debug("Executing Lua script: {}", luaCommand);

		auto state = xxlib::luavm::create();
		xxlib::luavm::add_json_library(state);
		xxlib::luavm::add_cpr_library(state);

		push_as_table(state, command.templateVars, "TEMPLATE_VARS");
		push_as_table(state, command.envs, "ENVS");
		push_as_table(state, positional, "POSITIONAL_EXTRAS");

		xxlib::luavm::new_table(state);
		{
			xxlib::luavm::push_string(state, "command_name");
			xxlib::luavm::push_string(state, command.name);
			xxlib::luavm::set_table(state, -3);

			xxlib::luavm::push_string(state, "os");
			xxlib::luavm::push_string(state, xxlib::platform::os_to_string(xxlib::platform::get_current_os()));
			xxlib::luavm::set_table(state, -3);

			xxlib::luavm::push_string(state, "arch");
			xxlib::luavm::push_string(state, xxlib::platform::architecture_to_string(xxlib::platform::get_current_architecture()));
			xxlib::luavm::set_table(state, -3);

			xxlib::luavm::push_string(state, "osfamily");
			xxlib::luavm::push_string(state, xxlib::platform::os_family_to_string(xxlib::platform::get_current_os_family()));
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
			spdlog::debug("Lua script returned nil, treating as successful no-op.");
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

			auto shellExecCommand = command;
			shellExecCommand.cmd = {shellCommand};
			shellExecCommand.executionEngine = xxlib::executor::Engine::System;
			shellExecCommand.requiresConfirmation = false;

			auto shellExecContext = CommandContext{
				.dryRun = false,
				.extras = {},
			};

			return xxlib::platform_executor::execute_command(shellExecCommand, shellExecContext);

		} else {
			return std::unexpected("Lua script returned unsupported type");
		}
	} // namespace lua_executor
} // namespace xxlib::lua_executor
