#ifndef XX_PLATFORM_HPP
#define XX_PLATFORM_HPP

#include <string>

namespace xxlib {
	namespace platform {
		enum class OSFamily { Windows, Unix, Unknown };
		inline OSFamily string_to_os_family(const std::string& osFamily) {
			if (osFamily == "windows") {
				return OSFamily::Windows;
			} else if (osFamily == "unix") {
				return OSFamily::Unix;
			} else {
				return OSFamily::Unknown;
			}
		}

		enum class OS { Windows, MacOS, Linux, Unknown };
		inline OS string_to_os(const std::string& os) {
			if (os == "windows") {
				return OS::Windows;
			} else if (os == "macos") {
				return OS::MacOS;
			} else if (os == "linux") {
				return OS::Linux;
			} else {
				return OS::Unknown;
			}
		}

		enum class Architecture { x86_64, arm64, unknown };
		inline Architecture string_to_architecture(const std::string& arch) {
			if (arch == "x86_64") {
				return Architecture::x86_64;
			} else if (arch == "arm64") {
				return Architecture::arm64;
			} else {
				return Architecture::unknown;
			}
		}

		OSFamily get_current_os_family();
		OS get_current_os();
		Architecture get_current_architecture();
	} // namespace platform
} // namespace xxlib

#endif // XX_PLATFORM_HPP
