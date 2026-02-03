#ifndef XX_COMMAND_HPP
#define XX_COMMAND_HPP

#include "detail/renderer.hpp"
#include "detail/executor.hpp"
#include <string>
#include <vector>
#include <unordered_map>

struct Command {
	std::string name{};
	std::vector<std::string> cmd{};
	std::unordered_map<std::string, std::string> templateVars{};
	std::unordered_map<std::string, std::string> envs{};
	std::vector<std::pair<std::string, std::string>> constraints{};

	xxlib::renderer::Engine renderEngine = xxlib::renderer::Engine::None;
	xxlib::executor::Engine executionEngine = xxlib::executor::Engine::System;

	bool requiresConfirmation = false;

	bool userScope = false;
};

struct CommandContext {
	bool dryRun = false;
	std::vector<std::string> extras{};
};

namespace xxlib::command {
	std::string render(const std::string& templateStr, const std::unordered_map<std::string, std::string>& templateVars, xxlib::renderer::Engine renderEngine);
	std::string join_cmd(const Command& command);
	std::string join_constraints(const Command& command);
} // namespace xxlib::command

#endif // XX_COMMAND_HPP
