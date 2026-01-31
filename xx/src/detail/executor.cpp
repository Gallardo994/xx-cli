#include "detail/executor.hpp"

#include <cstdlib>
#include <array>
#include <memory>
#include <string>

namespace xxlib::executor {
	ExecutionResult execute_command(const Command& command) {
		ExecutionResult result{};
		std::string fullCommand;

		for (const auto& [key, value] : command.envs) {
			fullCommand += key + "=" + value + " ";
		}
		for (const auto& arg : command.cmd) {
			fullCommand += arg + " ";
		}

		std::string output;
		std::string errorOutput;

		std::shared_ptr<FILE> pipe(popen(fullCommand.c_str(), "r"), pclose);
		if (!pipe) {
			result.exitCode = -1;
			errorOutput = "popen() failed!";
			result.errorOutput = errorOutput;
			return result;
		}

		std::array<char, 128> buffer;
		while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
			output += buffer.data();
		}

		auto returnCode = pclose(pipe.get());
		
		// TODO: Unsupported on Windows
		result.exitCode = WEXITSTATUS(returnCode);
		result.output = output;
		result.errorOutput = errorOutput;

		return result;
	}
} // namespace xxlib::executor
