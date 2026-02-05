#ifndef XX_PLATFORM_EXECUTOR_HPP
#define XX_PLATFORM_EXECUTOR_HPP

#include "detail/command.hpp"
#include <string>
#include <cstdint>
#include <expected>

namespace xxlib::platform_executor {
	[[nodiscard]] std::expected<int32_t, std::string> execute_command(Command& command, CommandContext& context);
} // namespace xxlib::platform_executor

#endif // XX_PLATFORM_EXECUTOR_HPP
