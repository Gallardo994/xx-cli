#include "detail/command.hpp"
#include "third_party/inja.hpp"

#include <numeric>
#include <string>
#include <spdlog/spdlog.h>

namespace xxlib {
	namespace command {
		CommandRenderEngine string_to_command_render_engine(const std::string& rendererStr) {
			if (rendererStr == "inja") {
				return CommandRenderEngine::Inja;
			} else if (rendererStr == "none") {
				return CommandRenderEngine::None;
			} else {
				throw std::invalid_argument("Unknown renderer type: " + rendererStr);
			}
		}

		CommandExecutionEngine string_to_command_execution_engine(const std::string& executorStr) {
			if (executorStr == "system") {
				return CommandExecutionEngine::System;
			} else if (executorStr == "lua") {
				return CommandExecutionEngine::Lua;
			} else {
				throw std::invalid_argument("Unknown executor type: " + executorStr);
			}
		}

		template <typename T> std::string join_vector(const std::vector<T>& vec, const std::string& separator) {
			if (vec.empty()) {
				return "";
			}

			return std::accumulate(std::next(vec.begin()), vec.end(), vec[0], [&separator](const std::string& a, const T& b) {
				return a + separator + b;
			});
		}

		std::string render_inja_template(const std::string& templateStr, const std::unordered_map<std::string, std::string>& templateVars) {
			inja::json data;
			for (const auto& [key, value] : templateVars) {
				data[key] = value;
			}
			return inja::render(templateStr, data);
		}

		std::string render(const std::string& templateStr, const std::unordered_map<std::string, std::string>& templateVars, CommandRenderEngine renderEngine) {
			if (renderEngine == CommandRenderEngine::Inja) {
				return render_inja_template(templateStr, templateVars);
			} else {
				return templateStr;
			}
		}

		std::string join_cmd(const Command& command) {
			return join_vector(command.cmd, " ");
		}

		std::string join_constraints(const Command& command) {
			if (command.constraints.empty()) {
				return "";
			}

			std::vector<std::string> parts;
			parts.reserve(command.constraints.size());

			for (const auto& [key, value] : command.constraints) {
				parts.push_back(key + "=" + value);
			}

			return join_vector(parts, ", ");
		}
	} // namespace command
} // namespace xxlib
