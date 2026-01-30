#include "detail/planner.hpp"

namespace xxlib::planner {
	bool matches_constraints(const Command& command) {
		for (const auto& [key, value] : command.constraints) {
			if (key == "os") {
#ifdef _WIN32
				const std::string currentOS = "windows";
#elif __APPLE__
				const std::string currentOS = "macos";
#elif __linux__
				const std::string currentOS = "linux";
#else
				const std::string currentOS = "unknown";
#endif
				if (currentOS != value) {
					return false;
				}
			} else if (key == "arch") {
#ifdef __x86_64__
				const std::string currentArch = "x86_64";
#elif __aarch64__
				const std::string currentArch = "arm64";
#else
				const std::string currentArch = "unknown";
#endif
				if (currentArch != value) {
					return false;
				}
			} else if (key == "osfamily") {
#ifdef _WIN32
				const std::string currentOSFamily = "windows";
#elif __APPLE__
				const std::string currentOSFamily = "unix";
#elif __linux__
				const std::string currentOSFamily = "unix";
#else
				const std::string currentOSFamily = "unknown";
#endif
				if (currentOSFamily != value) {
					return false;
				}
			} else {
				return false;
			}
		}

		return true;
	}

	std::optional<Command> plan_single(const std::vector<Command>& commands, const std::string& commandName) {
		for (const auto& command : commands) {
			if (command.name != commandName) {
				continue;
			}

			if (matches_constraints(command)) {
				return command;
			}
		}

		return std::nullopt;
	}
} // namespace xxlib::planner
