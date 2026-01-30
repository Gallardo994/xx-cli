#ifndef XX_PLANNER_HPP
#define XX_PLANNER_HPP

#include "detail/parser.hpp"

namespace xxlib {
	namespace planner {
		std::optional<Command> plan_single(const std::vector<Command>& commands, const std::string& commandName);
	} // namespace planner
} // namespace xxlib

#endif // XX_PLANNER_HPP
