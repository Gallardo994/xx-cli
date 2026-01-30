#include "xxlib.hpp"
#include "third_party/CLI11.hpp"

#include <iostream>
#include <filesystem>

struct GlobalArgs {
	std::string configFile;
	bool upFlag = false;
	bool verboseFlag = false;
	bool dryRunFlag = false;
};

namespace {
	std::optional<std::string> find_config(const std::string& startPath, const GlobalArgs& globalArgs) {
		if (globalArgs.upFlag) {
			std::filesystem::path currentPath = startPath;

			try {
				while (true) {
					std::filesystem::path configPath = currentPath / globalArgs.configFile;
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
			std::filesystem::path configPath = std::filesystem::path(startPath) / globalArgs.configFile;
			if (std::filesystem::exists(configPath)) {
				return configPath.string();
			}
		}

		return std::nullopt;
	}
} // namespace

int main(int argc, char** argv) {
	CLI::App app{"xx – Per‑project alias & preset tool"};
	GlobalArgs globalArgs{};

	app.add_option("-c,--config", globalArgs.configFile, "Path to project configuration file")->default_val(".xx.toml");
	app.add_flag("--up", globalArgs.upFlag, "Instead of using the current directory to locate the project configuration file, search parent directories");
	app.add_flag("-v,--verbose", globalArgs.verboseFlag, "Enable verbose output");
	app.add_flag("-n,--dry", globalArgs.dryRunFlag, "Perform a dry run without executing commands, act like they succeeded");

	const auto workdir = std::filesystem::current_path().string();

	app.add_subcommand("version", "Show version information")->callback([&]() { std::cout << "xx version " << XXLIB_VERSION << std::endl; });

	app.add_subcommand("list", "List all available commands")->callback([&]() {
		auto configPathOpt = find_config(workdir, globalArgs);
		if (!configPathOpt) {
			std::cerr << "Configuration file not found." << std::endl;
			return;
		}

		const auto buffer = xxlib::parser::read_file(*configPathOpt);
		if (!buffer) {
			std::cerr << "Error reading configuration file: " << buffer.error() << std::endl;
			return;
		}

		auto parseResult = xxlib::parser::parse_buffer(*buffer);
		if (!parseResult) {
			std::cerr << "Error parsing configuration: " << parseResult.error() << std::endl;
			return;
		}

		const auto& commands = *parseResult;
		for (const auto& cmd : commands) {
			const auto joinedCmd = std::accumulate(std::next(cmd.cmd.begin()), cmd.cmd.end(), cmd.cmd[0], [](const std::string& a, const std::string& b) { return a + " " + b; });
			std::cout << cmd.name << ": " << joinedCmd << std::endl;
		}
	});

	int exitCode = 0;

	auto* run = app.add_subcommand("run", "Run a specified command");
	std::string commandName;
	run->add_option("command", commandName, "Name of the command to run")->required();
	run->allow_extras();
	run->callback([&]() {
		auto configPathOpt = find_config(workdir, globalArgs);
		if (!configPathOpt) {
			std::cerr << "Configuration file not found." << std::endl;
			return;
		}

		const auto buffer = xxlib::parser::read_file(*configPathOpt);
		if (!buffer) {
			std::cerr << "Error reading configuration file: " << buffer.error() << std::endl;
			return;
		}

		auto parseResult = xxlib::parser::parse_buffer(*buffer);
		if (!parseResult) {
			std::cerr << "Error parsing configuration: " << parseResult.error() << std::endl;
			return;
		}

		const auto& commands = *parseResult;
		auto plannedCommandOpt = xxlib::planner::plan_single(commands, commandName);
		if (!plannedCommandOpt) {
			std::cerr << "No suitable command found for the current environment." << std::endl;
			return;
		}

		auto& commandToRun = *plannedCommandOpt;

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

		auto execResult = xxlib::executor::execute_command(commandToRun);

		if (globalArgs.verboseFlag) {
			std::cout << "Command executed with exit code: " << execResult.exitCode << std::endl;
			std::cout << "Output: " << execResult.output << std::endl;
			if (!execResult.errorOutput.empty()) {
				std::cerr << "Error Output: " << execResult.errorOutput << std::endl;
			}
		} else {
			std::cout << execResult.output;
		}

		exitCode = execResult.exitCode;
	});

	CLI11_PARSE(app, argc, argv);

	return exitCode;
}
