#ifndef XX_RENDERER
#define XX_RENDERER

#include <string>
#include <unordered_map>

namespace xxlib::renderer {
	enum class Engine {
		None,
		Inja,
	};

	xxlib::renderer::Engine string_to_render_engine(const std::string& rendererStr);

	std::string render(const std::string& templateStr, const std::unordered_map<std::string, std::string>& templateVars, Engine renderEngine);
} // namespace xxlib::renderer

#endif // XX_RENDERER
