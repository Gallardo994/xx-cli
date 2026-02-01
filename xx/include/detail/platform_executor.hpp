#ifndef XX_PLATFORM_EXECUTOR_HPP
#define XX_PLATFORM_EXECUTOR_HPP

#include "detail/command.hpp"
#include <string>
#include <cstdint>
#include <expected>

namespace xxlib {
	namespace platform_executor {
		std::string build_shell_command(const Command& command);
		std::expected<int32_t, std::string> execute_command(const Command& command);
	} // namespace platform_executor
} // namespace xxlib

#endif // XX_PLATFORM_EXECUTOR_HPP
