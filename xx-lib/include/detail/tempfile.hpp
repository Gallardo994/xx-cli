#ifndef XX_TEMPFILE_HPP
#define XX_TEMPFILE_HPP

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <system_error>
#include <array>

#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <cstring>
#include <limits.h>
#endif

namespace xxlib {
	struct TempFile {
		std::string path;

		explicit TempFile(const std::string& fileSuffix = "", bool thisDirectory = false) {
			const auto tempDir = thisDirectory ? std::filesystem::current_path().string() : std::filesystem::temp_directory_path().string();
			spdlog::debug("Using temporary directory: {}", tempDir);

#ifdef _WIN32
			std::array<char, MAX_PATH> tmpName{};
			if (GetTempFileNameA(tempDir.c_str(), "/xxcli", 0, tmpName.data()) == 0) {
				throw std::runtime_error("Failed to create temporary file (WinAPI)");
			}

			path = std::string(tmpName.data()) + fileSuffix;
			std::filesystem::rename(tmpName.data(), path);

			spdlog::debug("Created temporary file {}", path);
#else
			std::string templateFile = tempDir + "/xxcliXXXXXX";

			auto* tmpl_c = strdup(templateFile.c_str());
			if (!tmpl_c) {
				throw std::runtime_error("strdup failed while creating temp file template");
			}

			int fd = mkstemp(tmpl_c);
			if (fd == -1) {
				const std::string err = std::strerror(errno);
				free(tmpl_c);
				throw std::runtime_error("mkstemp failed: " + err);
			}
			close(fd);

			if (!fileSuffix.empty()) {
				const std::string newPath = std::string(tmpl_c) + fileSuffix;
				std::error_code ec;
				std::filesystem::rename(tmpl_c, newPath, ec);
				if (ec) {
					free(tmpl_c);
					throw std::runtime_error("Failed to rename temporary file: " + ec.message());
				}
				path = std::move(newPath);
			} else {
				path = tmpl_c;
			}

			free(tmpl_c);

			spdlog::debug("Created temporary file {}", path);
#endif
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

		TempFile(TempFile&& other) noexcept : path(std::move(other.path)) {
			other.path.clear();
		}

		TempFile& operator=(TempFile&& other) noexcept {
			if (this != &other) {
				this->~TempFile();

				path = std::move(other.path);
				other.path.clear();
			}
			return *this;
		}
	};
} // namespace xxlib

#endif // XX_TEMPFILE_HPP
