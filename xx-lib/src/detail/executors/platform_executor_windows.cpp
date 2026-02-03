#include "detail/executors/platform_executor.hpp"
#include "detail/helpers.hpp"
#include "detail/renderer.hpp"

#include <windows.h>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <array>
#include <spdlog/spdlog.h>

namespace xxlib::platform_executor {
	struct TempFile {
		std::string path;

		TempFile() {
			auto tempDir = std::filesystem::temp_directory_path().string();

			std::array<char, MAX_PATH> tempFileName{};
			if (GetTempFileNameA(tempDir.c_str(), "xxcli", 0, tempFileName.data()) == 0) {
				throw std::runtime_error("Failed to create temporary file");
			}

			path = std::string(tempFileName.data()) + ".ps1";
			std::filesystem::rename(std::string(tempFileName.data()), path);

			spdlog::debug("Created temporary file {}", path);
		}

		~TempFile() {
			if (!path.empty() && std::filesystem::exists(path)) {
				std::error_code ec;
				std::filesystem::remove(path, ec);
				if (ec) {
					spdlog::warn("Failed to delete temporary file {}: {}", path, ec.message());
				} else {
					spdlog::debug("Deleted temporary file {}", path);
				}
			}
		}

		TempFile(const TempFile&) = delete;
		TempFile& operator=(const TempFile&) = delete;
	};

	std::string build_shell_command(const Command& command) {
		std::ostringstream oss;
		for (const auto& [key, value] : command.envs) {
			oss << "$env:" << key << "=\"" << value << "\"; ";
		}
		for (const auto& arg : command.cmd) {
			oss << xxlib::renderer::render(arg, command.templateVars, command.renderEngine) << " ";
		}
		return oss.str();
	}

	std::expected<int32_t, std::string> execute_command(Command& command, CommandContext& context) {
		if (command.renderEngine == xxlib::renderer::Engine::None) {
			spdlog::debug("Using simple append for extra arguments (no rendering).");

			command.cmd.insert(command.cmd.end(), context.extras.begin(), context.extras.end());
		} else {
			spdlog::debug("Using template variable rendering for extra arguments.");

			auto extrasResult = xxlib::helpers::split_extras(context.extras);
			for (const auto& [key, value] : extrasResult.kv) {
				if (command.templateVars.find(key) != command.templateVars.end()) {
					command.templateVars[key] = value;
					spdlog::debug("Setting template variable: {}={}", key, value);
					continue;
				}
			}

			for (const auto& positional : extrasResult.positional) {
				spdlog::debug("Adding positional extra argument: {}", positional);
				command.cmd.push_back(positional);
			}
		}

		const auto unsetVars = xxlib::helpers::get_uset_vars(command.templateVars);
		if (!unsetVars.empty()) {
			std::ostringstream errOss;
			errOss << "The following template variables are not set: ";
			for (size_t i = 0; i < unsetVars.size(); ++i) {
				errOss << unsetVars[i];
				if (i < unsetVars.size() - 1) {
					errOss << ", ";
				}
			}
			return std::unexpected(errOss.str());
		}

		auto fullCommand = build_shell_command(command);

		if (context.dryRun) {
			spdlog::info("Command to be executed: {}", fullCommand);
			return 0;
		}

		if (command.requiresConfirmation) {
			if (xxlib::helpers::ask_for_confirmation("System Executor Engine wants to run: \n" + fullCommand)) {
				spdlog::debug("User confirmed execution.");
			} else {
				spdlog::debug("User denied execution.");
				return 0;
			}
		}

		spdlog::debug("Executing system command: {}", fullCommand);

		// Recommended by https://en.cppreference.com/w/cpp/utility/program/system.html
		std::cout << std::flush;

		auto tempFile = TempFile();
		std::ofstream ofs(tempFile.path);
		if (!ofs) {
			return std::unexpected("Failed to create temporary script file");
		}
		ofs << fullCommand;
		ofs.close();

		auto cmd = "powershell -ExecutionPolicy Bypass -File \"" + tempFile.path + "\"";
		auto returnCode = std::system(cmd.c_str());
		spdlog::debug("Command exited with return code: {}", returnCode);
		if (returnCode == -1) {
			return std::unexpected("Failed to execute command");
		}

		return static_cast<int32_t>(returnCode);
	}
} // namespace xxlib::platform_executor
