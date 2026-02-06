#include "xxlib.hpp"

#include <fmt/core.h>
#include <curl/curl.h>

namespace xxlib {
	std::string version() {
		return "1.1.1";
	}

	std::string detailed_version_text() {
		const auto xxlibVersion = fmt::format("xxlib version {} on {} {}",
											  xxlib::version(),
											  xxlib::platform::os_to_string(xxlib::platform::get_current_os()),
											  xxlib::platform::architecture_to_string(xxlib::platform::get_current_architecture()));
		const auto luaVersion = fmt::format("lua version {}", xxlib::luavm::version());
		const auto curlVersion = fmt::format("cURL version {}", curl_version());

		return fmt::format("{}\n{}\n{}", xxlibVersion, luaVersion, curlVersion);
	}
} // namespace xxlib
