#include "detail/executor.hpp"
#include "detail/executors/dotnet_run_executor.hpp"
#include "detail/executors/platform_executor.hpp"
#include "detail/command.hpp"
#include "detail/helpers.hpp"
#include "detail/renderer.hpp"
#include "detail/tempfile.hpp"

#include <expected>
#include <fstream>
#include <string>
#include <spdlog/spdlog.h>

namespace xxlib::dotnet_run_executor {
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

		std::string dotnetCommand;
		for (const auto& part : command.cmd) {
			dotnetCommand += xxlib::renderer::render(part, command.templateVars, command.renderEngine) + " ";
		}

		if (context.dryRun) {
			spdlog::info("Dotnet file to be executed: {}", dotnetCommand);
			return 0;
		}

		if (command.requiresConfirmation) {
			if (xxlib::helpers::ask_for_confirmation("Dotnet Run Executor Engine wants to run: \n" + dotnetCommand)) {
				spdlog::debug("User confirmed execution.");
			} else {
				spdlog::debug("User denied execution.");
				return 0;
			}
		}

		spdlog::debug("Executing Dotnet Run command: {}", dotnetCommand);

		auto tempFile = TempFile(".cs", true);
		spdlog::debug("Created temporary dotnet file at path: {}", tempFile.path);

		std::ofstream ofs(tempFile.path);
		if (!ofs) {
			return std::unexpected("Failed to create temporary dotnet file: " + tempFile.path);
		}
		ofs << dotnetCommand;
		ofs.close();

		auto shellExecCommand = command;
		shellExecCommand.cmd = {"dotnet run --file", tempFile.path};
		shellExecCommand.executionEngine = xxlib::executor::Engine::System;
		shellExecCommand.requiresConfirmation = false;

		auto shellExecContext = CommandContext{
			.dryRun = false,
			.extras = {},
		};

		return xxlib::platform_executor::execute_command(shellExecCommand, shellExecContext);
	} // namespace lua_executor
} // namespace xxlib::dotnet_run_executor
