#ifndef XX_RENDERER_HPP
#define XX_RENDERER_HPP

#include <string>
#include <unordered_map>

namespace xxlib::renderer {
	enum class Engine {
		None,
		Inja,
	};

	[[nodiscard]] xxlib::renderer::Engine string_to_render_engine(const std::string& rendererStr);

	[[nodiscard]] std::string render(const std::string& templateStr, const std::unordered_map<std::string, std::string>& templateVars, Engine renderEngine);
} // namespace xxlib::renderer

#endif // XX_RENDERER_HPP
