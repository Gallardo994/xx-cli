#include "detail/renderer.hpp"
#include <gtest/gtest.h>

TEST(Renderer_StringToRenderEngine, Inja) {
	auto engine = xxlib::renderer::string_to_render_engine("inja");
	EXPECT_EQ(engine, xxlib::renderer::Engine::Inja);
}

TEST(Renderer_StringToRenderEngine, None) {
	auto engine = xxlib::renderer::string_to_render_engine("none");
	EXPECT_EQ(engine, xxlib::renderer::Engine::None);
}

TEST(Renderer_StringToRenderEngine, Unknown) {
	EXPECT_THROW(xxlib::renderer::string_to_render_engine("unknown"), std::invalid_argument);
}

TEST(Renderer_Render, InjaEngine) {
	const auto templateStr = "echo \"{{ greeting }}, {{ target }}!\"";
	const auto templateVars = std::unordered_map<std::string, std::string>{
		{"greeting", "Hello"},
		{"target", "World"},
	};

	auto result = xxlib::renderer::render(templateStr, templateVars, xxlib::renderer::Engine::Inja);
	EXPECT_EQ(result, "echo \"Hello, World!\"");
}

TEST(Renderer_Render, NoneEngine) {
	const auto templateStr = "echo \"{{ greeting }}, {{ target }}!\"";
	const auto templateVars = std::unordered_map<std::string, std::string>{
		{"greeting", "Hello"},
		{"target", "World"},
	};

	auto result = xxlib::renderer::render(templateStr, templateVars, xxlib::renderer::Engine::None);
	EXPECT_EQ(result, "echo \"{{ greeting }}, {{ target }}!\"");
}
