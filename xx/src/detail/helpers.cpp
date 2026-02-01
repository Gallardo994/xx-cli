#include "detail/helpers.hpp"
#include <iostream>

namespace xxlib::helpers {
	bool ask_for_confirmation(const std::string& text) {
		std::cout << text;
		auto symbol = std::getc(stdin);
		return std::tolower(symbol) == 'y';
	}
} // namespace xxlib::helpers
