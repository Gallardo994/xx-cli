#include "detail/planner.hpp"
#include "detail/platform.hpp"

#include <functional>

namespace xxlib::planner {
	bool matches_constraints(const Command& command) {
		for (const auto& [key, value] : command.constraints) {
			if (key == "os") {
				if (xxlib::platform::string_to_os(value) != xxlib::platform::get_current_os()) {
					return false;
				}
			} else if (key == "arch") {
				if (xxlib::platform::string_to_architecture(value) != xxlib::platform::get_current_architecture()) {
					return false;
				}
			} else if (key == "osfamily") {
				if (xxlib::platform::string_to_os_family(value) != xxlib::platform::get_current_os_family()) {
					return false;
				}
			} else {
				return false;
			}
		}

		return true;
	}

	std::expected<Command, std::string> plan_single(const std::vector<Command>& commands, const std::string& commandName) {
		std::vector<std::reference_wrapper<const Command>> matchedCommands;

		for (const auto& command : commands) {
			if (command.name != commandName) {
				continue;
			}

			if (matches_constraints(command)) {
				matchedCommands.push_back(std::cref(command));
			}
		}

		if (matchedCommands.size() == 1) {
			return matchedCommands.front().get();
		} else if (matchedCommands.size() > 1) {
			return std::unexpected("Multiple matching commands found for name: " + commandName);
		}

		return std::unexpected("No matching command found for name: " + commandName);
	}
} // namespace xxlib::planner
