#include "detail/platform.hpp"

namespace xxlib {
	namespace platform {
		OSFamily get_current_os_family() {
#if _WIN32
			return OSFamily::Windows;
#elif __APPLE__
			return OSFamily::Unix;
#elif __linux__
			return OSFamily::Unix;
#else
            static assert(false, "Unsupported OS detected");
#endif
		}

		OS get_current_os() {
#if _WIN32
			return OS::Windows;
#elif __APPLE__
			return OS::MacOS;
#elif __linux__
			return OS::Linux;
#else
            static_assert(false, "Unsupported OS detected");
#endif
		}

		Architecture get_current_architecture() {
#if __x86_64__ || _M_X64
			return Architecture::x86_64;
#elif __aarch64__ || _M_ARM64
			return Architecture::arm64;
#else
            static_assert(false, "Unsupported architecture detected");
#endif
		}
	} // namespace platform
} // namespace xxlib
