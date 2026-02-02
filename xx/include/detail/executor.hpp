#ifndef XX_EXECUTOR_HPP
#define XX_EXECUTOR_HPP

#include "detail/command.hpp"

#include <string>
#include <expected>
#include <cstdint>

namespace xxlib {
	namespace executor {
		std::expected<int32_t, std::string> execute_command(Command& command, CommandContext& context);
	} // namespace executor
} // namespace xxlib

#endif // XX_EXECUTOR_HPP
