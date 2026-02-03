#ifndef XX_HELPERS_HPP
#define XX_HELPERS_HPP

#include <string>
#include <unordered_map>
#include <vector>

namespace xxlib::helpers {
	struct ExtrasResult {
		std::unordered_map<std::string, std::string> kv;
		std::vector<std::string> positional;
	};

	bool ask_for_confirmation(const std::string& text);
	ExtrasResult split_extras(const std::vector<std::string>& extras);
	std::vector<std::string> get_uset_vars(const std::unordered_map<std::string, std::string>& templateVars);
} // namespace xxlib::helpers

#endif // XX_HELPERS_HPP
