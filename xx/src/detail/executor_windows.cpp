#include "detail/executor.hpp"

#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <array>

namespace xxlib::executor {
	std::string build_powershell(const Command& command) {
		// TODO: Very much unsafe. Needs better escaping.
		const auto escape_arg = [](const std::string& arg) -> std::string {
			std::string escaped;
			for (char c : arg) {
				if (c == '"') {
					escaped += "\\\"";
				} else {
					escaped += c;
				}
			}
			return escaped;
		};

		std::ostringstream oss;
		oss << "powershell.exe -NoProfile -Command \"& { ";
		for (const auto& [key, value] : command.envs) {
			oss << "$env:" << key << "=\\\"" << escape_arg(value) << "\\\"; ";
		}
		for (const auto& arg : command.cmd) {
			oss << escape_arg(arg) << " ";
		}
		oss << " }\"";
		return oss.str();
	}

	std::expected<int32_t, std::string> execute_command(const Command& command, bool verbose) {
		auto fullCommand = build_powershell(command);

		if (verbose) {
			std::cout << "Executing system command: " << fullCommand << std::endl;
		}

		SECURITY_ATTRIBUTES sa{};
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;

		HANDLE stdoutRead = nullptr;
		HANDLE stdoutWrite = nullptr;

		if (!CreatePipe(&stdoutRead, &stdoutWrite, &sa, 0)) {
			return std::unexpected("CreatePipe() failed");
		}

		SetHandleInformation(stdoutRead, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOA si{};
		si.cb = sizeof(STARTUPINFOA);
		si.hStdOutput = stdoutWrite;
		// TODO: We should probably capture stderr separately so we don't modify original behavior of a command.
		si.hStdError = stdoutWrite;
		si.dwFlags |= STARTF_USESTDHANDLES;

		PROCESS_INFORMATION pi{};

		auto success = CreateProcessA(nullptr, fullCommand.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);

		CloseHandle(stdoutWrite);

		if (!success) {
			CloseHandle(stdoutRead);
			return std::unexpected("CreateProcessA() failed");
		}

		std::array<char, 4096> buffer;
		DWORD bytesRead = 0;

		while (true) {
			auto success = ReadFile(stdoutRead, buffer.data(), buffer.size(), &bytesRead, nullptr);
			if (!success || bytesRead == 0) {
				break;
			}

			std::cout.write(buffer.data(), bytesRead);
			std::cout.flush();
		}

		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exitCode = 0;
		GetExitCodeProcess(pi.hProcess, &exitCode);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(stdoutRead);

		return static_cast<int32_t>(exitCode);
	}
} // namespace xxlib::executor
