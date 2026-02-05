#ifndef XX_INJA_RENDERER_HPP
#define XX_INJA_RENDERER_HPP

#include <string>
#include <unordered_map>

namespace xxlib::inja_renderer {
	[[nodiscard]] std::string render(const std::string& templateStr, const std::unordered_map<std::string, std::string>& templateVars);
} // namespace xxlib::inja_renderer

#endif // XX_INJA_RENDERER_HPP
