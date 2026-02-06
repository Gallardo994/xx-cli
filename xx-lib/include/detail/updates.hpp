#ifndef XX_UPDATES_HPP
#define XX_UPDATES_HPP

#include <string>
#include <optional>
#include <expected>

namespace xxlib::updates {
	struct ReleaseInfo {
		std::string version;
		std::string url;
	};

	[[nodiscard]] std::expected<std::optional<ReleaseInfo>, std::string> get_latest_release();
} // namespace xxlib::updates

#endif // XX_UPDATES_HPP
