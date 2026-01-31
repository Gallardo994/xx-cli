#include "xxlib.hpp"
#include "third_party/CLI11.hpp"

#include <cstdio>
#include <filesystem>
#include <sstream>
#include <string>
#include <optional>
#include <spdlog/spdlog.h>

struct GlobalArgs {
	std::string configFile;
	std::string userConfigFile;
	bool upFlag = false;
	bool verboseFlag = false;
	bool dryRunFlag = false;
};

namespace {
	std::optional<std::string> find_config(const std::string& startPath, const std::string_view configFile, bool upFlag) {
		if (upFlag) {
			std::filesystem::path currentPath = startPath;

			std::string configFileName{configFile};
			if (const auto path = std::filesystem::path(configFile); path.has_parent_path()) {
				configFileName = path.filename().string();
			}

			try {
				while (true) {
					std::filesystem::path configPath = currentPath / configFile;
					if (std::filesystem::exists(configPath)) {
						return configPath.string();
					}

					if (currentPath.has_parent_path() && currentPath != currentPath.parent_path()) {
						currentPath = currentPath.parent_path();
					} else {
						break;
					}
				}
			} catch (const std::filesystem::filesystem_error& e) {
				spdlog::error("Filesystem error while searching for config: {}", e.what());
				return std::nullopt;
			}
		} else {
			std::filesystem::path configPath = std::filesystem::path(startPath) / configFile;
			if (std::filesystem::exists(configPath)) {
				return configPath.string();
			}
		}

		return std::nullopt;
	}

	std::vector<Command> load_commands(const GlobalArgs& globalArgs, const std::string& workdir) {
		spdlog::debug("Loading configuration from workdir: {}", workdir);

		std::vector<Command> commands;

		auto configPathOpt = find_config(workdir, globalArgs.configFile, globalArgs.upFlag);
		if (configPathOpt) {
			spdlog::debug("Configuration file found at: {}", *configPathOpt);

			const auto buffer = xxlib::parser::read_file(*configPathOpt);
			if (buffer) {
				auto parseResult = xxlib::parser::parse_buffer(*buffer);
				if (!parseResult) {
					throw std::runtime_error("Error parsing configuration: " + parseResult.error());
				}

				commands.insert(commands.end(), parseResult->begin(), parseResult->end());
				spdlog::debug("Loaded {} project commands.", parseResult->size());
			} else {
				spdlog::debug("Error reading configuration file: {}", buffer.error());
			}
		} else {
			spdlog::debug("No configuration file found.");
		}

		const auto userConfigFilePath = std::filesystem::path(globalArgs.userConfigFile);
		const auto userBuffer = xxlib::parser::read_file(userConfigFilePath.string());
		if (userBuffer) {
			spdlog::debug("User configuration found: {}", userConfigFilePath.string());

			auto userParseResult = xxlib::parser::parse_buffer(*userBuffer);
			if (userParseResult) {
				auto& userCommands = *userParseResult;
				for (auto& cmd : userCommands) {
					cmd.userScope = true;
				}

				commands.insert(commands.end(), userCommands.begin(), userCommands.end());
				spdlog::debug("Loaded {} user commands.", userCommands.size());
			} else {
				spdlog::debug("Error parsing user configuration: {}", userParseResult.error());
			}
		} else {
			spdlog::debug("No user configuration file found at: {}", userConfigFilePath.string());
		}

		return commands;
	}
} // namespace

int main(int argc, char** argv) {
	CLI::App app{"xx – Per‑project alias & preset tool"};
	GlobalArgs globalArgs{};

	app.add_option("-c,--config", globalArgs.configFile, "Path to project configuration file")->default_val(".xx.toml");
	app.add_option("-u,--user-config", globalArgs.userConfigFile, "Path to user configuration file")
#ifdef _WIN32
		->default_val(std::getenv("APPDATA") + std::string("\\xx\\xx.toml"));
#elif __APPLE__ || __linux__
		->default_val(std::getenv("HOME") + std::string("/.config/xx/xx.toml"));
#else
		static_assert(false, "Unsupported platform");
#endif
	app.add_flag("--up", globalArgs.upFlag, "Instead of using the current directory to locate the project configuration file, search parent directories");
	app.add_flag("-v,--verbose", globalArgs.verboseFlag, "Enable verbose output");
	app.add_flag("-n,--dry", globalArgs.dryRunFlag, "Perform a dry run without executing commands, act like they succeeded");

	app.parse_complete_callback([&]() {
		if (globalArgs.verboseFlag) {
			spdlog::set_level(spdlog::level::debug);
			spdlog::set_pattern("[%^%l%$] %v");
		} else {
			spdlog::set_level(spdlog::level::info);
			spdlog::set_pattern("%v");
		}
	});

	const auto workdir = std::filesystem::current_path().string();

	app.add_subcommand("version", "Show version information")->callback([&]() {
		spdlog::info("xxlib version {}", XXLIB_VERSION);
		spdlog::info("lua version {}", LUA_VERSION);
	});

	auto* list = app.add_subcommand("list", "List all available commands");
	std::string listGrep;
	list->add_option("--grep", listGrep, "Filter commands by name containing the specified substring");
	list->callback([&]() {
		const auto user_tag = [](const Command& cmd) -> std::string {
			return cmd.userScope ? "[User] " : "";
		};

		const auto commands = load_commands(globalArgs, workdir);

		std::vector<std::string> constraintSatisfiedCommands;
		std::vector<std::string> constraintUnsatisfiedCommands;
		for (const auto& cmd : commands) {
			auto cmdText = xxlib::command::join_cmd(cmd);

			if (!listGrep.empty() && (cmd.name.find(listGrep) == std::string::npos && cmdText.find(listGrep) == std::string::npos)) {
				continue;
			}

			std::ostringstream fullText;
			fullText << user_tag(cmd) << cmd.name << ": " << cmdText;

			if (xxlib::planner::matches_constraints(cmd)) {
				constraintSatisfiedCommands.push_back(fullText.str());
			} else {
				fullText << " [Constraints: " << xxlib::command::join_constraints(cmd) << "]";
				constraintUnsatisfiedCommands.push_back(fullText.str());
			}
		}

		spdlog::info("Commands available for the current environment:");
		for (const auto& str : constraintSatisfiedCommands) {
			spdlog::info("-- {}", str);
		}

		if (!constraintUnsatisfiedCommands.empty()) {
			spdlog::info("Commands not available for the current environment: (due to constraints)");
			for (const auto& str : constraintUnsatisfiedCommands) {
				spdlog::info("-- {}", str);
			}
		}
	});

	app.add_subcommand("user-config-path", "Show the path to the user configuration file")->callback([&]() {
		spdlog::info("{}", globalArgs.userConfigFile);
	});

	int32_t exitCode = -1;

	auto* run = app.add_subcommand("run", "Run a specified command");
	std::string commandName;
	bool yoloFlag = false;
	run->add_option("command", commandName, "Name of the command to run")->required();
	run->add_flag("--yolo", yoloFlag, "Run the command without confirmation, even if it requires confirmation");
	run->allow_extras();
	run->callback([&]() {
		const auto commands = load_commands(globalArgs, workdir);

		auto plannedCommand = xxlib::planner::plan_single(commands, commandName);
		if (!plannedCommand.has_value()) {
			spdlog::error("Error planning command '{}': {}", commandName, plannedCommand.error());
			return;
		}

		auto& commandToRun = plannedCommand.value();

		const auto& extras = run->remaining();
		if (!extras.empty()) {
			spdlog::debug("Appending {} extra arguments to command.", extras.size());
			for (const auto& extra : extras) {
				spdlog::debug("  Extra argument: {}", extra);
			}

			if (commandToRun.executionEngine == CommandExecutionEngine::System) {
				if (commandToRun.renderEngine == CommandRenderEngine::None) {
					spdlog::debug("Using simple append for extra arguments (no rendering).");

					commandToRun.cmd.insert(commandToRun.cmd.end(), extras.begin(), extras.end());
				} else {
					spdlog::debug("Using template variable rendering for extra arguments.");

					for (const auto& extra : extras) {
						auto pos = extra.find('=');
						if (pos != std::string::npos) {
							auto key = extra.substr(0, pos);
							if (commandToRun.templateVars.find(key) != commandToRun.templateVars.end()) {
								auto value = extra.substr(pos + 1);
								commandToRun.templateVars[key] = value;

								spdlog::debug("Adding template variable: {}={}", key, value);
								continue;
							}
						}

						spdlog::debug("Adding simple append variable (not part of template_vars / k=v format): {}", extra);
						commandToRun.cmd.push_back(extra);
					}
				}
			} else if (commandToRun.executionEngine == CommandExecutionEngine::Lua) {
				for (const auto& extra : extras) {
					auto pos = extra.find('=');
					if (pos != std::string::npos) {
						auto key = extra.substr(0, pos);
						auto value = extra.substr(pos + 1);
						commandToRun.templateVars[key] = value;

						spdlog::debug("Adding template variable: {}={}", key, value);
					} else {
						spdlog::debug("Skipping non-kv variable: {}", extra);
					}
				}
			}
		}

		if (globalArgs.dryRunFlag) {
			exitCode = 0;
			spdlog::info("Dry run: Command to be executed: {}", xxlib::executor::build_shell_command(commandToRun));
			return;
		}

		if (commandToRun.requiresConfirmation && !yoloFlag) {
			spdlog::info("Command to be executed: {}", xxlib::executor::build_shell_command(commandToRun));
			spdlog::info("Type 'y' to confirm execution:");

			auto symbol = std::getc(stdin);
			if (std::tolower(symbol) != 'y') {
				spdlog::info("Command execution cancelled by user.");
				exitCode = 0;
				return;
			}
		}

		std::expected<int32_t, std::string> execResult;

		if (commandToRun.executionEngine == CommandExecutionEngine::System) {
			execResult = xxlib::executor::execute_command(commandToRun);
		} else if (commandToRun.executionEngine == CommandExecutionEngine::Lua) {
			auto state = xxlib::luavm::create();
			std::string luaCommand;

			for (const auto& part : commandToRun.cmd) {
				luaCommand += xxlib::command::render(part, commandToRun.templateVars, commandToRun.renderEngine) + " ";
			}
			spdlog::debug("Executing Lua script: {}", luaCommand);

			const auto push_as_table = [&](const std::unordered_map<std::string, std::string>& map, const char* tableName) {
				xxlib::luavm::new_table(state);
				for (const auto& [key, value] : map) {
					xxlib::luavm::push_string(state, key);
					xxlib::luavm::push_string(state, value);
					xxlib::luavm::set_table(state, -3);
				}
				xxlib::luavm::set_global(state, tableName);
			};

			push_as_table(commandToRun.templateVars, "TEMPLATE_VARS");
			push_as_table(commandToRun.envs, "ENVS");

			xxlib::luavm::new_table(state);
			{
				xxlib::luavm::push_string(state, "command_name");
				xxlib::luavm::push_string(state, commandToRun.name);
				xxlib::luavm::set_table(state, -3);

				xxlib::luavm::push_string(state, "is_dry_run");
				xxlib::luavm::push_boolean(state, globalArgs.dryRunFlag);
				xxlib::luavm::set_table(state, -3);
			}
			xxlib::luavm::set_global(state, "CTX");

			if (xxlib::luavm::loadstring(state, luaCommand) || xxlib::luavm::pcall(state, 0, 1, 0)) {
				execResult = std::unexpected(std::string("Error executing Lua command: ") + xxlib::luavm::tostring(state));
			}

			if (xxlib::luavm::is_nil(state)) {
				exitCode = 0;
				spdlog::info("Lua script returned nil, treating as successful no-op.");
			} else if (xxlib::luavm::is_integer(state)) {
				exitCode = static_cast<int32_t>(xxlib::luavm::tointeger(state));
				spdlog::debug("Lua script returned exit code: {}", exitCode);
			} else if (xxlib::luavm::is_boolean(state)) {
				bool boolResult = xxlib::luavm::toboolean(state);
				exitCode = boolResult ? 0 : 1;
				spdlog::debug("Lua script returned boolean: {}, treating as exit code: {}", boolResult, exitCode);
			} else if (xxlib::luavm::is_string(state)) {
				const auto* shellCommand = xxlib::luavm::tostring(state);
				spdlog::debug("Lua script returned shell command: {}", shellCommand);
				Command shellExecCommand = commandToRun;
				shellExecCommand.cmd = {shellCommand};
				shellExecCommand.executionEngine = CommandExecutionEngine::System;
				execResult = xxlib::executor::execute_command(shellExecCommand);
			} else {
				execResult = std::unexpected("Lua script returned unsupported type");
			}
		} else {
			execResult = std::unexpected("Unknown execution engine");
		}

		if (!execResult) {
			spdlog::error("Error executing command '{}': {}", commandName, execResult.error());
			return;
		}

		exitCode = execResult.value();
	});

	CLI11_PARSE(app, argc, argv);

	return exitCode;
}
