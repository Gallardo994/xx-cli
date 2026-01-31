#include "detail/executor.hpp"

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

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

	std::expected<int32_t, std::string> execute_command(const Command& command, bool verbose) {
		auto fullCommand = build_shell(command);

		if (verbose) {
			std::cout << "Executing system command: " << fullCommand << std::endl;
		}

		auto returnCode = std::system(fullCommand.c_str());
		if (returnCode == -1) {
			return std::unexpected("Failed to execute command");
		}

		return WEXITSTATUS(returnCode);
	}
} // namespace xxlib::executor
