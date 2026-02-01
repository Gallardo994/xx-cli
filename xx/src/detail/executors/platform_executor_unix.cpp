#include "detail/executors/platform_executor.hpp"
#include "detail/helpers.hpp"

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <spdlog/spdlog.h>

namespace xxlib::platform_executor {
	std::string build_shell_command(const Command& command) {
		std::ostringstream oss;
		for (const auto& [key, value] : command.envs) {
			oss << key << "=" << value << " ";
		}
		for (const auto& arg : command.cmd) {
			oss << command::render(arg, command.templateVars, command.renderEngine) << " ";
		}
		return oss.str();
	}

	std::expected<int32_t, std::string> execute_command(const Command& command, bool dryRun) {
		auto fullCommand = build_shell_command(command);

		if (dryRun) {
			spdlog::info("Command to be executed: {}", fullCommand);
			return 0;
		}

		if (command.requiresConfirmation) {
			if (xxlib::helpers::ask_for_confirmation("System Executor Engine wants to run (y/n): " + fullCommand)) {
				spdlog::info("User confirmed execution.");
			} else {
				spdlog::info("User denied execution.");
				return 0;
			}
		}

		spdlog::debug("Executing system command: {}", fullCommand);

		// Recommended by https://en.cppreference.com/w/cpp/utility/program/system.html
		std::cout << std::flush;

		auto returnCode = std::system(fullCommand.c_str());
		spdlog::debug("Command exited with return code: {}", returnCode);
		if (returnCode == -1) {
			return std::unexpected("Failed to execute command");
		}

		return WEXITSTATUS(returnCode);
	}
} // namespace xxlib::platform_executor
