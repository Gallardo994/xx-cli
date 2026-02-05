#include "detail/renderers/inja_renderer.hpp"

#include <inja/inja.hpp>
#include <string>

namespace xxlib::inja_renderer {
	std::string render(const std::string& templateStr, const std::unordered_map<std::string, std::string>& templateVars) {
		inja::json data;

		for (const auto& [key, value] : templateVars) {
			data[key] = value;
		}

		return inja::render(templateStr, data);
	}
} // namespace xxlib::inja_renderer
