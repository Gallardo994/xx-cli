#include "detail/executor.hpp"

#include <cstdlib>
#include <array>
#include <string>
#include <sstream>
#include <iostream>

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

		auto pipe = popen(fullCommand.c_str(), "r");
		if (!pipe) {
			return std::unexpected("popen() failed");
		}

		std::array<char, 4096> buffer;
		size_t bytesRead = 0;

		while ((fgets(buffer.data(), buffer.size(), pipe))) {
			std::cout << buffer.data();
			std::cout.flush();
		}

		auto returnCode = pclose(pipe);
		return WEXITSTATUS(returnCode);
	}
} // namespace xxlib::executor
