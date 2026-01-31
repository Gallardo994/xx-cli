#include "xxlib.hpp"
#include "third_party/CLI11.hpp"

#include <iostream>
#include <filesystem>
#include <string>
#include <optional>

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

			try {
				while (true) {
					std::filesystem::path configPath = currentPath / configFile;
					if (std::filesystem::exists(configPath)) {
						return configPath.string();
					}

					if (currentPath.has_parent_path()) {
						currentPath = currentPath.parent_path();
					} else {
						break;
					}
				}
			} catch (const std::filesystem::filesystem_error& e) {
				std::cerr << "Filesystem error: " << e.what() << std::endl;
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
		if (globalArgs.verboseFlag) {
			std::cout << "Loading configuration from workdir: " << workdir << std::endl;
		}

		auto configPathOpt = find_config(workdir, globalArgs.configFile, globalArgs.upFlag);
		if (!configPathOpt) {
			throw std::runtime_error("Configuration file not found.");
		}

		const auto buffer = xxlib::parser::read_file(*configPathOpt);
		if (!buffer) {
			throw std::runtime_error("Error reading configuration file: " + buffer.error());
		}

		auto parseResult = xxlib::parser::parse_buffer(*buffer, globalArgs.verboseFlag);
		if (!parseResult) {
			throw std::runtime_error("Error parsing configuration: " + parseResult.error());
		}

		auto& commands = *parseResult;

		const auto userConfigFilePath = std::filesystem::path(globalArgs.userConfigFile);
		const auto userBuffer = xxlib::parser::read_file(userConfigFilePath.string());
		if (userBuffer) {
			if (globalArgs.verboseFlag) {
				std::cout << "User configuration found: " << userConfigFilePath.string() << std::endl;
			}

			auto userParseResult = xxlib::parser::parse_buffer(*userBuffer, globalArgs.verboseFlag);
			if (userParseResult) {
				auto& userCommands = *userParseResult;
				for (auto& cmd : userCommands) {
					cmd.userScope = true;
				}

				commands.insert(commands.end(), userCommands.begin(), userCommands.end());

				if (globalArgs.verboseFlag) {
					std::cout << "Loaded " << userCommands.size() << " user commands." << std::endl;
				}
			} else {
				if (globalArgs.verboseFlag) {
					std::cout << "Error parsing user configuration: " << userParseResult.error() << std::endl;
				}
			}
		} else {
			if (globalArgs.verboseFlag) {
				std::cout << "User configuration file not found or could not be read: " << userBuffer.error() << std::endl;
			}
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

	const auto workdir = std::filesystem::current_path().string();

	app.add_subcommand("version", "Show version information")->callback([&]() { std::cout << "xx version " << XXLIB_VERSION << std::endl; });

	app.add_subcommand("list", "List all available commands")->callback([&]() {
		auto configPathOpt = find_config(workdir, globalArgs.configFile, globalArgs.upFlag);
		if (!configPathOpt) {
			std::cerr << "Configuration file not found." << std::endl;
			return;
		}

		const auto commands = load_commands(globalArgs, workdir);

		std::vector<Command> constraintSatisfiedCommands;
		std::vector<Command> constraintUnsatisfiedCommands;
		for (const auto& cmd : commands) {
			if (xxlib::planner::matches_constraints(cmd)) {
				constraintSatisfiedCommands.push_back(cmd);
			} else {
				constraintUnsatisfiedCommands.push_back(cmd);
			}
		}

		const auto userTag = [](const Command& cmd) { return cmd.userScope ? "[User] " : ""; };

		std::cout << "Commands available for the current environment:" << std::endl;
		for (const auto& cmd : constraintSatisfiedCommands) {
			std::cout << userTag(cmd) << cmd.name << ": " << xxlib::command::join_cmd(cmd) << std::endl;
		}

		if (!constraintUnsatisfiedCommands.empty()) {
			std::cout << "\nCommands not available for the current environment (due to constraints):" << std::endl;
			for (const auto& cmd : constraintUnsatisfiedCommands) {
				std::cout << userTag(cmd) << cmd.name << ": " << xxlib::command::join_cmd(cmd) << " [Constraints: " << xxlib::command::join_constraints(cmd) << "]" << std::endl;
			}
		}
	});

	int32_t exitCode = -1;

	auto* run = app.add_subcommand("run", "Run a specified command");
	std::string commandName;
	run->add_option("command", commandName, "Name of the command to run")->required();
	run->allow_extras();
	run->callback([&]() {
		auto configPathOpt = find_config(workdir, globalArgs.configFile, globalArgs.upFlag);
		if (!configPathOpt) {
			std::cerr << "Configuration file not found." << std::endl;
			return;
		}

		const auto commands = load_commands(globalArgs, workdir);

		auto plannedCommand = xxlib::planner::plan_single(commands, commandName);
		if (!plannedCommand.has_value()) {
			std::cerr << "Error planning command: " << plannedCommand.error() << std::endl;
			return;
		}

		auto& commandToRun = plannedCommand.value();

		const auto& extras = run->remaining();
		if (!extras.empty()) {
			commandToRun.cmd.insert(commandToRun.cmd.end(), extras.begin(), extras.end());
		}

		if (globalArgs.dryRunFlag) {
			exitCode = 0;

			std::cout << "Dry run: Command to be executed: ";
			for (const auto& arg : commandToRun.cmd) {
				std::cout << arg << " ";
			}
			std::cout << std::endl;

			return;
		}

		auto execResult = xxlib::executor::execute_command(commandToRun, globalArgs.verboseFlag);

		if (!execResult) {
			if (globalArgs.verboseFlag) {
				std::cerr << "Error executing command: " << execResult.error() << std::endl;
			}

			return;
		}

		exitCode = execResult.value();
	});

	CLI11_PARSE(app, argc, argv);

	return exitCode;
}
