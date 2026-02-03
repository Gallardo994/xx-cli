#include "detail/parser.hpp"
#include "detail/renderer.hpp"
#include <yaml-cpp/yaml.h>

#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

namespace xxlib::parser {
	std::expected<std::string, std::string> read_file(const std::string& path) {
		try {
			std::ifstream file(path);
			if (!file.is_open()) {
				return std::unexpected("Failed to open file: " + path);
			}

			file.seekg(0, std::ios::end);
			std::streamsize size = file.tellg();
			if (size > 1024 * 1024) {
				return std::unexpected("File size exceeds 1 MB limit: " + path);
			}
			file.seekg(0, std::ios::beg);

			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			return content;
		} catch (const std::exception& e) {
			return std::unexpected(std::string("Error reading file: ") + e.what());
		}
	}

	std::expected<Command, std::string> parse_command(const YAML::Node& node) {
		Command command;

		try {
			if (!node["cmd"]) {
				return std::unexpected("Missing 'cmd' field");
			}

			const auto& cmdNode = node["cmd"];
			if (cmdNode.IsScalar()) {
				command.cmd.emplace_back(cmdNode.as<std::string>());
			} else if (cmdNode.IsSequence()) {
				for (const auto& item : cmdNode) {
					if (!item.IsScalar()) {
						return std::unexpected("Invalid element inside 'cmd' array – must be a string");
					}

					command.cmd.emplace_back(item.as<std::string>());
				}
			} else {
				return std::unexpected("'cmd' must be either a scalar or an array of scalars");
			}

			if (auto renderEngine = node["render_engine"]; renderEngine && renderEngine.IsScalar()) {
				command.renderEngine = xxlib::renderer::string_to_render_engine(renderEngine.as<std::string>());
			} else if (renderEngine) {
				return std::unexpected("'render_engine' must be a string");
			}

			if (auto executionEngine = node["execution_engine"]; executionEngine && executionEngine.IsScalar()) {
				command.executionEngine = xxlib::executor::string_to_execution_engine(executionEngine.as<std::string>());
			} else if (executionEngine) {
				return std::unexpected("'execution_engine' must be a string");
			}

			if (auto templateVars = node["template_vars"]; templateVars && templateVars.IsMap()) {
				for (const auto& kv : templateVars) {
					if (!kv.second.IsScalar() && !kv.second.IsNull()) {
						return std::unexpected("All values in 'template_vars' must be strings or null");
					}

					auto valueStr = kv.second.IsNull() ? "" : kv.second.as<std::string>();
					command.templateVars.emplace(kv.first.as<std::string>(), valueStr);
				}
			} else if (templateVars) {
				return std::unexpected("'template_vars' must be a map");
			}

			if (auto env = node["env"]; env && env.IsMap()) {
				for (const auto& kv : env) {
					if (!kv.second.IsScalar()) {
						return std::unexpected("All values in 'env' must be strings");
					}

					command.envs.emplace(kv.first.as<std::string>(), kv.second.as<std::string>());
				}
			} else if (env) {
				return std::unexpected("'env' must be a map");
			}

			if (auto constraints = node["constraints"]; constraints && constraints.IsSequence()) {
				for (auto i = 0; i < constraints.size(); ++i) {
					const auto& item = constraints[i];

					if (!item.IsMap() || item.size() != 1) {
						return std::unexpected("Each constraint must be a map with a single key/value pair");
					}

					const auto pair = item.begin();

					const auto key = pair->first;
					const auto value = pair->second;

					if (!key.IsScalar() || !value.IsScalar()) {
						return std::unexpected("Constraint keys and values must be strings");
					}

					command.constraints.emplace_back(key.Scalar(), value.Scalar());
				}
			} else if (constraints) {
				return std::unexpected("'constraints' must be a sequence");
			}

			if (auto requiresConfirmation = node["requires_confirmation"]; requiresConfirmation && requiresConfirmation.IsScalar()) {
				const auto raw = requiresConfirmation.Scalar();
				if (raw == "true") {
					command.requiresConfirmation = true;
				} else if (raw == "false") {
					command.requiresConfirmation = false;
				} else {
					return std::unexpected("'requires_confirmation' must be a boolean (true/false)");
				}
			} else if (requiresConfirmation) {
				return std::unexpected("'requires_confirmation' must be a boolean");
			}
		} catch (const YAML::Exception& e) {
			return std::unexpected(std::string("YAML parsing error: ") + e.what());
		} catch (const std::exception& e) {
			return std::unexpected(std::string("Error parsing command: ") + e.what());
		}

		if (command.cmd.empty()) {
			return std::unexpected("Command 'cmd' cannot be empty");
		}

		return command;
	}

	std::expected<std::vector<Command>, std::string> parse_buffer(const std::string& buffer) {
		try {
			const YAML::Node root = YAML::Load(buffer);
			if (!root.IsMap()) {
				return std::unexpected("Root of the config must be a map");
			}

			const auto aliasNode = root["alias"];
			if (!aliasNode) {
				const auto flatNode = root["aliases"];
				if (!flatNode || !flatNode.IsSequence()) {
					return std::unexpected("No 'alias' (or 'aliases') section found in config");
				}

				std::vector<Command> cmds;

				for (const auto& entry : flatNode) {
					if (!entry.IsMap()) {
						return std::unexpected("Each element of 'aliases' must be a map");
					}

					const auto nameNode = entry["name"];
					if (!nameNode || !nameNode.IsScalar()) {
						return std::unexpected("Alias entry is missing a string 'name' field");
					}

					const auto aliasName = nameNode.as<std::string>();

					auto opt = parse_command(entry);
					if (!opt) {
						return std::unexpected(opt.error());
					}

					auto cmd = *opt;
					cmd.name = aliasName;
					cmds.emplace_back(std::move(cmd));
				}
				return cmds;
			}

			std::vector<Command> commands;

			for (const auto& aliasPair : aliasNode) {
				const auto aliasName = aliasPair.first.as<std::string>();
				const auto aliasVal = aliasPair.second;

				if (aliasVal.IsMap()) {
					auto opt = parse_command(aliasVal);
					if (!opt) {
						return std::unexpected(opt.error());
					}

					auto cmd = *opt;
					cmd.name = aliasName;
					commands.emplace_back(std::move(cmd));
				} else if (aliasVal.IsSequence()) {
					for (const auto& item : aliasVal) {
						if (!item.IsMap()) {
							return std::unexpected("Each element of alias." + aliasName + " must be a table");
						}

						auto opt = parse_command(item);
						if (!opt) {
							return std::unexpected(opt.error());
						}

						auto cmd = *opt;
						cmd.name = aliasName;
						commands.emplace_back(std::move(cmd));
					}
				} else {
					return std::unexpected("Alias '" + aliasName + "' must be a table or an array of tables");
				}
			}

			return commands;
		} catch (const YAML::ParserException& e) {
			return std::unexpected(std::string("YAML parse error: ") + e.what());
		} catch (const std::exception& e) {
			return std::unexpected(std::string("Error parsing buffer: ") + e.what());
		}
	}

} // namespace xxlib::parser
