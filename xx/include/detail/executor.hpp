#ifndef XX_EXECUTOR_HPP
#define XX_EXECUTOR_HPP

#include "detail/parser.hpp"

#include <string>

namespace xxlib {
	struct ExecutionResult {
		int exitCode;
		std::string output;
		std::string errorOutput;
	};

	namespace executor {
		ExecutionResult execute_command(const Command& command);
	} // namespace executor
} // namespace xxlib

#endif // XX_EXECUTOR_HPP
