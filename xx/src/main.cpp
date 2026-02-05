#include "xxlib.hpp"
#include "third_party/CLI11.hpp"

#include <cstdio>
#include <filesystem>
#include <sstream>
#include <string>
#include <optional>
#include <spdlog/spdlog.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <semver.hpp>

struct GlobalArgs {
	std::string configFile;
	std::string userConfigFile;
	bool upFlag = false;
	bool verboseFlag = false;
	bool userConfigOnlyFlag = false;
	bool projectOnlyFlag = false;
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

	void load_project_configuration(std::vector<Command>& commands, const std::string& configFile, const bool upFlag, const std::string& workdir) {
		spdlog::debug("Loading configuration from workdir: {}", workdir);

		auto configPathOpt = find_config(workdir, configFile, upFlag);
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
	}

	void load_user_configuration(std::vector<Command>& commands, const std::string& userConfigFile) {
		const auto userConfigFilePath = std::filesystem::path(userConfigFile);
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
	}

	std::vector<Command> load_commands(const GlobalArgs& globalArgs, const std::string& workdir) {
		std::vector<Command> commands;

		if (!globalArgs.userConfigOnlyFlag) {
			load_project_configuration(commands, globalArgs.configFile, globalArgs.upFlag, workdir);
		}

		if (!globalArgs.projectOnlyFlag) {
			load_user_configuration(commands, globalArgs.userConfigFile);
		}

		return commands;
	}
} // namespace

int main(int argc, char** argv) {
	CLI::App app{"xx – Per‑project alias & preset tool"};
	GlobalArgs globalArgs{};

	app.add_option("-c,--config", globalArgs.configFile, "Path to project configuration file")->default_val(".xx.yaml");
	app.add_option("-u,--user-config", globalArgs.userConfigFile, "Path to user configuration file")
#ifdef _WIN32
		->default_val(std::getenv("APPDATA") + std::string("\\xx\\xx.yaml"));
#elif __APPLE__ || __linux__
		->default_val(std::getenv("HOME") + std::string("/.config/xx/xx.yaml"));
#else
		static_assert(false, "Unsupported platform");
#endif
	app.add_flag("--up", globalArgs.upFlag, "Instead of using the current directory to locate the project configuration file, search parent directories");
	app.add_flag("-v,--verbose", globalArgs.verboseFlag, "Enable verbose output");
	app.add_flag("--user", globalArgs.userConfigOnlyFlag, "Load only user configuration, ignoring project configuration");
	app.add_flag("--project", globalArgs.projectOnlyFlag, "Load only project configuration, ignoring user configuration");

	app.parse_complete_callback([&]() {
		if (globalArgs.verboseFlag) {
			spdlog::set_level(spdlog::level::debug);
			spdlog::set_pattern("[%^%l%$] %v");
		} else {
			spdlog::set_level(spdlog::level::info);
			spdlog::set_pattern("%v");
		}

		if (globalArgs.userConfigOnlyFlag && globalArgs.projectOnlyFlag) {
			spdlog::error("Cannot use both --user and --project flags at once as they are mutually exclusive.");
			std::exit(1);
		}

		if (globalArgs.projectOnlyFlag && globalArgs.upFlag) {
			spdlog::warn("The --up flag has no effect when --project flag is used.");
		}
	});

	const auto workdir = std::filesystem::current_path().string();

	app.add_subcommand("version", "Show version information")->callback([&]() {
		spdlog::info("xxlib version {} on {} {}",
					 xxlib::version(),
					 xxlib::platform::os_to_string(xxlib::platform::get_current_os()),
					 xxlib::platform::architecture_to_string(xxlib::platform::get_current_architecture()));
		spdlog::info("lua version {}", xxlib::luavm::version());
	});

	app.add_subcommand("check-updates", "Check for updates")->callback([&]() {
		semver::version current;
		if (!semver::parse(xxlib::version(), current)) {
			spdlog::error("Failed to parse current version: {}", xxlib::version());
			return;
		}

		const auto latestUrlApi = "https://api.github.com/repos/gallardo994/xx-cli/releases/latest";
		const auto latestUrlPage = "https://github.com/gallardo994/xx-cli/releases/latest";

		try {
			auto response = cpr::Get(cpr::Url{latestUrlApi});
			if (response.status_code != 200) {
				spdlog::error("Failed to check for updates. HTTP status code: {}", response.status_code);
				return;
			}

			auto jsonResponse = nlohmann::json::parse(response.text);
			auto latestVersion = jsonResponse["tag_name"].get<std::string>();

			semver::version latest;
			if (!semver::parse(latestVersion, latest)) {
				spdlog::error("Failed to parse latest version from response: {}", latestVersion);
				return;
			}

			if (latest > current) {
				spdlog::info("A new version of xx is available: {} (current: {})", latestVersion, xxlib::version());
				spdlog::info("Visit {} to download the latest version.", latestUrlPage);

				if (xxlib::helpers::ask_for_confirmation("Do you want to open the releases page in your default browser?")) {
#if defined(_WIN32)
					std::string command = "start " + std::string(latestUrlPage);
					std::system(command.c_str());
#elif defined(__APPLE__)
					std::string command = "open " + std::string(latestUrlPage);
					std::system(command.c_str());
#elif defined(__linux__)
					std::string command = "xdg-open " + std::string(latestUrlPage);
					std::system(command.c_str());
#else
					spdlog::error("Opening the browser is not supported on this platform.");
#endif
				}
			} else {
				spdlog::info("You are using the latest version of xx: {}", xxlib::version());
			}
		} catch (const std::exception& ex) {
			spdlog::error("Error while checking for updates: {}", ex.what());
		}
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
	bool dryRunFlag = false;
	run->add_option("command", commandName, "Name of the command to run")->required();
	run->add_flag("-y,--yolo", yoloFlag, "Run the command without confirmation, even if it requires confirmation");
	run->add_flag("-n,--dry", dryRunFlag, "Perform a dry run without executing commands, act like they succeeded");
	run->allow_extras();
	run->callback([&]() {
		const auto commands = load_commands(globalArgs, workdir);

		auto plannedCommand = xxlib::planner::plan_single(commands, commandName);
		if (!plannedCommand.has_value()) {
			spdlog::error("Error planning command '{}': {}", commandName, plannedCommand.error());
			return;
		}

		auto& commandToRun = plannedCommand.value();

		if (yoloFlag) {
			commandToRun.requiresConfirmation = false;
			spdlog::debug("yolo flag is set, requiresConfirmation will be ignored.");
		}

		auto execContext = CommandContext{
			.dryRun = dryRunFlag,
			.extras = run->remaining(),
		};

		auto execResult = xxlib::executor::execute_command(commandToRun, execContext);

		if (!execResult) {
			spdlog::error("Error executing command '{}': {}", commandName, execResult.error());
			return;
		}

		exitCode = execResult.value();
	});

	CLI11_PARSE(app, argc, argv);

	return exitCode;
}
