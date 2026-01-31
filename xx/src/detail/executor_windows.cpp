#include "detail/executor.hpp"

#include <windows.h>
#include <sstream>
#include <iostream>

namespace xxlib::executor {
	std::string build_powershell(const Command& command) {
		// TODO: Very much unsafe. Needs better escaping.
		const auto escape_arg = [](const std::string& arg) -> std::string {
			std::string escaped;
			for (char c : arg) {
				if (c == '"') {
					escaped += "\\\"";
				} else {
					escaped += c;
				}
			}
			return escaped;
		};

		std::ostringstream oss;
		oss << "powershell.exe -NoProfile -Command \"& { ";
		for (const auto& [key, value] : command.envs) {
			oss << "$env:" << key << "=\\\"" << escape_arg(value) << "\\\"; ";
		}
		for (const auto& arg : command.cmd) {
			oss << escape_arg(arg) << " ";
		}
		oss << " }\"";
		return oss.str();
	}

	std::expected<int32_t, std::string> execute_command(const Command& command, bool verbose) {
		auto fullCommand = build_powershell(command);

		if (verbose) {
			std::cout << "Executing system command: " << fullCommand << std::endl;
		}

		// Recommended by https://en.cppreference.com/w/cpp/utility/program/system.html
		std::cout << std::flush;

		auto returnCode = std::system(fullCommand.c_str());
		if (returnCode == -1) {
			return std::unexpected("Failed to execute command");
		}

		return static_cast<int32_t>(returnCode);
	}
} // namespace xxlib::executor
