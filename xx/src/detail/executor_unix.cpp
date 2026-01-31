#include "detail/executor.hpp"

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <spdlog/spdlog.h>

namespace xxlib::executor {
	std::string build_shell(const Command& command) {
		std::ostringstream oss;
		for (const auto& [key, value] : command.envs) {
			oss << key << "=" << value << " ";
		}
		for (const auto& arg : command.cmd) {
			oss << arg << " ";
		}
		return oss.str();
	}

	std::expected<int32_t, std::string> execute_command(const Command& command) {
		auto fullCommand = build_shell(command);
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
} // namespace xxlib::executor
