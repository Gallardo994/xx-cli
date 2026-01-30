#ifndef XX_PLATFORM_HPP
#define XX_PLATFORM_HPP

#include <string>

namespace xxlib {
	namespace platform {
		enum class OSFamily { windows, unix, unknown };
		inline OSFamily string_to_os_family(const std::string& osFamily) {
			if (osFamily == "windows") {
				return OSFamily::windows;
			} else if (osFamily == "unix") {
				return OSFamily::unix;
			} else {
				return OSFamily::unknown;
			}
		}

		enum class OS { windows, macos, linux, unknown };
		inline OS string_to_os(const std::string& os) {
			if (os == "windows") {
				return OS::windows;
			} else if (os == "macos") {
				return OS::macos;
			} else if (os == "linux") {
				return OS::linux;
			} else {
				return OS::unknown;
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
