#include "detail/renderers/inja_renderer.hpp"
#include <gtest/gtest.h>

TEST(InjaRenderer_Render, BasicRendering) {
	const auto templateStr = "echo \"{{ greeting }}, {{ target }}!\"";
	const auto templateVars = std::unordered_map<std::string, std::string>{
		{"greeting", "Hello"},
		{"target", "World"},
	};

	auto result = xxlib::inja_renderer::render(templateStr, templateVars);
	EXPECT_EQ(result, "echo \"Hello, World!\"");
}

TEST(InjaRenderer_Render, MissingVariable) {
	const auto templateStr = "echo \"{{ greeting }}, {{ target }}!\"";
	const auto templateVars = std::unordered_map<std::string, std::string>{
		{"greeting", "Hello"},
	};

	try {
		auto result = xxlib::inja_renderer::render(templateStr, templateVars);
		FAIL() << "Must be unreachable due to an exception";
	} catch (const std::runtime_error& e) {
		EXPECT_STREQ(e.what(), "[inja.exception.render_error] (at 1:26) variable 'target' not found");
	} catch (...) {
		FAIL() << "Expected std::runtime_error";
	}
}

TEST(InjaRenderer_Render, MultilineTemplate) {
	const auto templateStr = R"(
echo "Line 1: {{ line1 }}"
echo "Line 2: {{ line2 }}"
    )";
	const auto templateVars = std::unordered_map<std::string, std::string>{
		{"line1", "First"},
		{"line2", "Second"},
	};

	auto result = xxlib::inja_renderer::render(templateStr, templateVars);
	const auto expected = R"(
echo "Line 1: First"
echo "Line 2: Second"
    )";

	EXPECT_EQ(result, expected);
}

TEST(InjaRenderer_Render, NoVariables) {
	const auto templateStr = "echo \"No variables\"";
	const auto templateVars = std::unordered_map<std::string, std::string>{
		{"unused", "value"},
	};

	auto result = xxlib::inja_renderer::render(templateStr, templateVars);
	EXPECT_EQ(result, "echo \"No variables\"");
}
