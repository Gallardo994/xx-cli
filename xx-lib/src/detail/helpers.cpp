#include "detail/helpers.hpp"
#include <iostream>

namespace xxlib::helpers {
	bool ask_for_confirmation(const std::string& text) {
		std::cout << text << "\nConfirm (y/n): " << std::flush;
		auto symbol = std::getc(stdin);
		return std::tolower(symbol) == 'y';
	}

	ExtrasResult split_extras(const std::vector<std::string>& extras) {
		ExtrasResult result{};

		for (const auto& extra : extras) {
			auto pos = extra.find('=');
			if (pos != std::string::npos) {
				auto key = extra.substr(0, pos);
				if (key.empty()) {
					result.positional.push_back(extra);
					continue;
				}

				auto value = extra.substr(pos + 1);
				result.kv[key] = value;
			} else {
				result.positional.push_back(extra);
			}
		}

		return result;
	}
} // namespace xxlib::helpers
