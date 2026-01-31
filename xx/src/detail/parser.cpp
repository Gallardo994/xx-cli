#include "detail/parser.hpp"
#include "third_party/toml.hpp"

#include <iostream>

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

	std::expected<Command, std::string> parse_command(const toml::table& table) {
		Command command;

		try {
			if (auto cmdValue = table["cmd"]) {
				if (cmdValue.is_string()) {
					command.cmd.push_back(cmdValue.as_string()->get());
				} else if (cmdValue.is_array()) {
					for (const auto& item : *cmdValue.as_array()) {
						if (item.is_string()) {
							command.cmd.push_back(item.as_string()->get());
						} else {
							return std::unexpected("Invalid cmd array item type");
						}
					}
				} else {
					return std::unexpected("Invalid cmd type");
				}
			} else {
				return std::unexpected("Missing cmd field");
			}

			if (auto envsTable = table["env"].as_table()) {
				for (const auto& [key, value] : *envsTable) {
					if (value.is_string()) {
						command.envs.emplace(key, value.as_string()->get());
					} else {
						return std::unexpected("Invalid env value type for key");
					}
				}
			}

			if (auto constraintsArray = table["constraints"].as_array()) {
				for (const auto& item : *constraintsArray) {
					if (item.is_array() && item.as_array()->size() == 2) {
						const auto& pair = *item.as_array();
						if (pair[0].is_string() && pair[1].is_string()) {
							command.constraints.emplace_back(pair[0].as_string()->get(), pair[1].as_string()->get());
						} else {
							return std::unexpected("Invalid constraint pair types");
						}
					} else {
						return std::unexpected("Invalid constraint item type or size");
					}
				}
			}
		} catch (const std::exception& e) {
			return std::unexpected(std::string("Error parsing command: ") + e.what());
		}

		if (command.cmd.empty()) {
			return std::unexpected("Command 'cmd' cannot be empty");
		}

		return command;
	}

	std::expected<std::vector<Command>, std::string> parse_buffer(const std::string& buffer, bool verbose) {
		try {
			auto tomlData = toml::parse(buffer);
			std::vector<Command> commands;

			const auto add_command_from_table = [&](const std::string_view key, const toml::table& table) -> std::optional<std::string> {
				if (verbose) {
					std::cout << "Parsing command: " << key << std::endl;
				}

				auto commandOpt = parse_command(table);
				if (commandOpt) {
					Command command = *commandOpt;
					command.name = key;
					commands.push_back(command);
					return std::nullopt;
				} else {
					return commandOpt.error();
				}
			};

			if (auto aliasTable = tomlData["alias"].as_table()) {
				if (verbose) {
					std::cout << "Found " << aliasTable->size() << " command aliases in configuration." << std::endl;
				}

				commands.reserve(aliasTable->size());

				for (const auto& [key, value] : *aliasTable) {
					if (verbose) {
						std::cout << "Parsing command alias: " << key << " with type " << value.type() << std::endl;
					}

					if (value.is_table()) {
						auto errorOpt = add_command_from_table(key, *value.as_table());
						if (errorOpt) {
							return std::unexpected(*errorOpt);
						}
					} else if (value.is_array()) {
						for (const auto& item : *value.as_array()) {
							if (item.is_table()) {
								auto errorOpt = add_command_from_table(key, *item.as_table());
								if (errorOpt) {
									return std::unexpected(*errorOpt);
								}
							} else {
								return std::unexpected("Alias array item is not a table for key");
							}
						}
					}

					else {
						return std::unexpected("Alias entry is not a table for key");
					}
				}
			}

			return commands;
		} catch (const toml::parse_error& e) {
			return std::unexpected(std::string("TOML parse error: ") + e.what());
		} catch (const std::exception& e) {
			return std::unexpected(std::string("Error parsing buffer: ") + e.what());
		}
	};

} // namespace xxlib::parser
