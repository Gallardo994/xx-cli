#ifndef XX_DOTNET_RUN_EXECUTOR_HPP
#define XX_DOTNET_RUN_EXECUTOR_HPP

#include <string>
#include <cstdint>
#include <expected>

class Command;
class CommandContext;

namespace xxlib::dotnet_run_executor {
	[[nodiscard]] std::expected<int32_t, std::string> execute_command(Command& command, CommandContext& context);
} // namespace xxlib::dotnet_run_executor

#endif // XX_DOTNET_RUN_EXECUTOR_HPP
