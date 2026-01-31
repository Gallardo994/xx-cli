#ifndef XX_EXECUTOR_HPP
#define XX_EXECUTOR_HPP

#include "detail/command.hpp"
#include <string>
#include <cstdint>

namespace xxlib {
	struct ExecutionResult {
		int32_t exitCode = -1;
		std::string output{};
		std::string errorOutput{};
	};

	namespace executor {
		ExecutionResult execute_command(const Command& command);
	} // namespace executor
} // namespace xxlib

#endif // XX_EXECUTOR_HPP
