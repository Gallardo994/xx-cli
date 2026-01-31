#ifndef XX_COMMAND_HPP
#define XX_COMMAND_HPP

#include <string>
#include <vector>
#include <unordered_map>

enum class CommandRenderEngine {
	None,
	Inja,
};

struct Command {
	std::string name{};
	std::vector<std::string> cmd{};
	std::unordered_map<std::string, std::string> templateVars{};
	std::unordered_map<std::string, std::string> envs{};
	std::vector<std::pair<std::string, std::string>> constraints{};
	bool userScope = false;
	CommandRenderEngine renderEngine = CommandRenderEngine::None;
	bool requiresConfirmation = false;
};

namespace xxlib {
	namespace command {
		CommandRenderEngine string_to_command_render_engine(const std::string& rendererStr);
		std::string render(const std::string& templateStr, const std::unordered_map<std::string, std::string>& templateVars, CommandRenderEngine renderEngine);
		std::string join_cmd(const Command& command);
		std::string join_constraints(const Command& command);
	} // namespace command
} // namespace xxlib

#endif // XX_COMMAND_HPP
