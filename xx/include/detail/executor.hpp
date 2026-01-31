#ifndef XX_EXECUTOR_HPP
#define XX_EXECUTOR_HPP

#include "detail/command.hpp"
#include <string>
#include <cstdint>
#include <expected>

namespace xxlib {
	namespace executor {
		std::expected<int32_t, std::string> execute_command(const Command& command, bool verbose);
	} // namespace executor
} // namespace xxlib

#endif // XX_EXECUTOR_HPP
