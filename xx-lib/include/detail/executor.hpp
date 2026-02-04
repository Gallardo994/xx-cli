#ifndef XX_EXECUTOR_HPP
#define XX_EXECUTOR_HPP

#include <string>
#include <expected>
#include <cstdint>

class Command;
class CommandContext;

namespace xxlib::executor {
	enum class Engine {
		System,
		Lua,
		DotnetRun,
	};

	Engine string_to_execution_engine(const std::string& executorStr);

	std::expected<int32_t, std::string> execute_command(Command& command, CommandContext& context);
} // namespace xxlib::executor

#endif // XX_EXECUTOR_HPP
