#ifndef XX_PLATFORM_HPP
#define XX_PLATFORM_HPP

#include <string>

namespace xxlib::platform {
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
	inline std::string os_family_to_string(OSFamily osFamily) {
		switch (osFamily) {
		case OSFamily::Windows:
			return "windows";
		case OSFamily::Unix:
			return "unix";
		default:
			return "unknown";
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
	inline std::string os_to_string(OS os) {
		switch (os) {
		case OS::Windows:
			return "windows";
		case OS::MacOS:
			return "macos";
		case OS::Linux:
			return "linux";
		default:
			return "unknown";
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
	inline std::string architecture_to_string(Architecture arch) {
		switch (arch) {
		case Architecture::x86_64:
			return "x86_64";
		case Architecture::arm64:
			return "arm64";
		default:
			return "unknown";
		}
	}

	OSFamily get_current_os_family();
	OS get_current_os();
	Architecture get_current_architecture();
} // namespace xxlib::platform

#endif // XX_PLATFORM_HPP
