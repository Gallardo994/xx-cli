#include "detail/updates.hpp"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace xxlib::updates {
	std::expected<std::optional<ReleaseInfo>, std::string> get_latest_release() {
		const auto latestUrlApi = "https://api.github.com/repos/gallardo994/xx-cli/releases/latest";

		spdlog::debug("Checking for updates using GitHub API: {}", latestUrlApi);

		try {
			const auto response = cpr::Get(cpr::Url{latestUrlApi});
			if (response.status_code != 200) {
				spdlog::error("GitHub API request failed with status code {}: {}", response.status_code, response.text);
				return std::unexpected("Failed to check for updates: HTTP " + std::to_string(response.status_code));
			}

			const auto json = nlohmann::json::parse(response.text);
			spdlog::debug("GitHub API response: {}", json.dump(4));

			if (!json.contains("tag_name") || !json.contains("html_url")) {
				return std::unexpected("Invalid response from GitHub API");
			}

			auto info = ReleaseInfo{
				.version = json["tag_name"].get<std::string>(),
				.url = json["html_url"].get<std::string>(),
			};

			spdlog::debug("Latest release info: version={}, url={}", info.version, info.url);

			return info;
		} catch (const std::exception& e) {
			return std::unexpected(std::string("Failed to check for updates: ") + e.what());
		}
	}
} // namespace xxlib::updates
