#include "detail/renderer.hpp"
#include "detail/renderers/inja_renderer.hpp"

namespace xxlib::renderer {
	Engine string_to_render_engine(const std::string& rendererStr) {
		if (rendererStr == "inja") {
			return Engine::Inja;
		} else if (rendererStr == "none") {
			return Engine::None;
		} else {
			throw std::invalid_argument("Unknown renderer type: " + rendererStr);
		}
	}

	std::string render(const std::string& templateStr, const std::unordered_map<std::string, std::string>& templateVars, Engine renderEngine) {
		if (renderEngine == Engine::Inja) {
			return xxlib::inja_renderer::render(templateStr, templateVars);
		} else {
			return templateStr;
		}
	}
} // namespace xxlib::renderer
