#include "detail/executor.hpp"

#include <cstdlib>
#include <array>
#include <memory>
#include <string>

namespace xxlib::executor {
	ExecutionResult execute_command(const Command& command) {
		throw std::runtime_error("execute_command is not implemented for Windows yet.");
	}
} // namespace xxlib::executor
