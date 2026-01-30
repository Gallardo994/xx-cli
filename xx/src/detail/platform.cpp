#include "detail/platform.hpp"

namespace xxlib {
	namespace platform {
		OSFamily get_current_os_family() {
#ifdef _WIN32
			return OSFamily::windows;
#elif __APPLE__
			return OSFamily::unix;
#elif __linux__
			return OSFamily::unix;
#else
			return OSFamily::unknown;
#endif
		}

		OS get_current_os() {
#ifdef _WIN32
			return OS::windows;
#elif __APPLE__
			return OS::macos;
#elif __linux__
			return OS::linux;
#else
			return OS::unknown;
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
