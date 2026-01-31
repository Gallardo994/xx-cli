#include "detail/executor.hpp"

#include <cstdlib>
#include <stdexcept>

namespace xxlib::executor {
	ExecutionResult execute_command(const Command& command) {
		throw std::runtime_error("execute_command is not implemented for Windows yet.");
	}
} // namespace xxlib::executor
