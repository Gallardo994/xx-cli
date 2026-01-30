#include "detail/platform.hpp"

namespace xxlib {
	namespace platform {
		OSFamily get_current_os_family() {
#ifdef _WIN32
			return OSFamily::Windows;
#elif __APPLE__
			return OSFamily::Unix;
#elif __linux__
			return OSFamily::Unix;
#else
			return OSFamily::Unknown;
#endif
		}

		OS get_current_os() {
#ifdef _WIN32
			return OS::Windows;
#elif __APPLE__
			return OS::MacOS;
#elif __linux__
			return OS::Linux;
#else
			return OS::Unknown;
#endif
		}

		Architecture get_current_architecture() {
#ifdef __x86_64__
			return Architecture::x86_64;
#elif __aarch64__
			return Architecture::arm64;
#else
			return Architecture::unknown;
#endif
		}
	} // namespace platform
} // namespace xxlib
