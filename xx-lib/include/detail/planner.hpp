#ifndef XX_PLANNER_HPP
#define XX_PLANNER_HPP

#include "detail/command.hpp"
#include <vector>
#include <string>
#include <expected>

namespace xxlib::planner {
	[[nodiscard]] bool matches_constraints(const Command& command);
	[[nodiscard]] std::expected<Command, std::string> plan_single(const std::vector<Command>& commands, const std::string& commandName);
} // namespace xxlib::planner

#endif // XX_PLANNER_HPP
