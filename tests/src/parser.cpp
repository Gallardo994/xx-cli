#include "detail/parser.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <memory>

std::string create_test_file(const std::string& name, const std::string& content) {
	auto tempFilePath = std::filesystem::temp_directory_path() / name;
	std::ofstream outFile(tempFilePath, std::ios::binary);
	outFile << content;
	outFile.close();
	return tempFilePath.string();
}

struct TempFileDeleter {
	void operator()(const std::string* path) const {
		if (path && std::filesystem::exists(*path)) {
			std::filesystem::remove(*path);
		}
		delete path;
	}
};

struct TempFilePtr {
	std::unique_ptr<std::string, TempFileDeleter> path;

	TempFilePtr(const std::string& name, const std::string& content) : path(new std::string(create_test_file(name, content))) {
	}

	const std::string& get() const {
		return *path;
	}
};

TEST(Parser_ReadFile, FileExists) {
	auto tempFile = TempFilePtr("test_read_file_exists.txt", "Test content for read_file.");
	auto result = xxlib::parser::read_file(tempFile.get());

	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(result.value(), "Test content for read_file.");
}

TEST(Parser_ReadFile, FileDoesNotExist) {
	auto result = xxlib::parser::read_file("non_existent_file.txt");

	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "Failed to open file: non_existent_file.txt");
}

TEST(Parser_ReadFile, FileTooLarge) {
	auto largeContent = std::string(2 * 1024 * 1024, 'X'); // 2 MB content
	auto tempFile = TempFilePtr("test_read_file_too_large.txt", largeContent);
	auto result = xxlib::parser::read_file(tempFile.get());

	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "File size exceeds 1 MB limit: " + tempFile.get());
}

TEST(Parser_ParseBuffer, BasicYamlSingleCommand) {
	const auto yaml = R"(
alias:
  build:
    - cmd: "echo Building project"
)";

	auto result = xxlib::parser::parse_buffer(yaml);
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(result->size(), 1u);

	const auto& buildCmd = result->at(0);
	EXPECT_EQ(buildCmd.name, "build");
	ASSERT_EQ(buildCmd.cmd.size(), 1u);
	EXPECT_EQ(buildCmd.cmd.at(0), "echo Building project");
}

TEST(Parser_ParseBuffer, BasicYamlArray) {
	const auto yaml = R"(
alias:
  cmd1:
    - cmd: "echo Building project"
  cmd2:
    - cmd: "echo Running tests"
)";

	auto result = xxlib::parser::parse_buffer(yaml);
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(result->size(), 2u);

	const auto& cmd1 = result->at(0);
	EXPECT_EQ(cmd1.name, "cmd1");
	ASSERT_EQ(cmd1.cmd.size(), 1u);
	EXPECT_EQ(cmd1.cmd.at(0), "echo Building project");

	const auto& cmd2 = result->at(1);
	EXPECT_EQ(cmd2.name, "cmd2");
	ASSERT_EQ(cmd2.cmd.size(), 1u);
	EXPECT_EQ(cmd2.cmd.at(0), "echo Running tests");
}

TEST(Parser_ParseBuffer, YamlWithConstraints) {
	const auto yaml = R"(
alias:
  build:
    - cmd: "echo Building project"
      constraints:
        - os: linux
        - arch: x86_64

  test:
    - cmd: "echo Running tests"
      constraints:
        - osfamily: windows
)";

	auto result = xxlib::parser::parse_buffer(yaml);
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(result->size(), 2u);

	const auto& buildCmd = result->at(0);
	EXPECT_EQ(buildCmd.name, "build");
	ASSERT_EQ(buildCmd.cmd.size(), 1u);
	EXPECT_EQ(buildCmd.cmd.at(0), "echo Building project");
	ASSERT_EQ(buildCmd.constraints.size(), 2u);
	EXPECT_EQ(buildCmd.constraints.at(0).first, "os");
	EXPECT_EQ(buildCmd.constraints.at(0).second, "linux");
	EXPECT_EQ(buildCmd.constraints.at(1).first, "arch");
	EXPECT_EQ(buildCmd.constraints.at(1).second, "x86_64");

	const auto& testCmd = result->at(1);
	EXPECT_EQ(testCmd.name, "test");
	ASSERT_EQ(testCmd.cmd.size(), 1u);
	EXPECT_EQ(testCmd.cmd.at(0), "echo Running tests");
	ASSERT_EQ(testCmd.constraints.size(), 1u);
	EXPECT_EQ(testCmd.constraints.at(0).first, "osfamily");
	EXPECT_EQ(testCmd.constraints.at(0).second, "windows");
}

TEST(Parser_ParseBuffer, InvalidYaml) {
	const auto yaml = R"(
alias:
  invalid:
    - cmd: "echo oops"
      constraints: [ [ "os", "linux" ]
)";

	auto result = xxlib::parser::parse_buffer(yaml);
	ASSERT_FALSE(result.has_value());
	EXPECT_TRUE(result.error().find("YAML parse error") != std::string::npos);
}

TEST(Parser_ParseBuffer, MissingCmdField) {
	const std::string yaml = R"(
alias:
  empty_cmd: [ {} ]
)";

	auto result = xxlib::parser::parse_buffer(yaml);
	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "Missing 'cmd' field");
}

TEST(Parser_ParseBuffer, InvalidConstraintType) {
	const std::string yaml = R"(
alias:
  invalid_constraint:
    - cmd: "echo Testing invalid constraint"
      constraints:
        - not_a_pair
)";

	auto result = xxlib::parser::parse_buffer(yaml);
	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "Each constraint must be a map with a single key/value pair");
}

TEST(Parser_ParseBuffer, InvalidRequiresConfirmationType) {
	const std::string yaml = R"(
alias:
  invalid_confirmation:
    - cmd: "echo Testing invalid confirmation"
      requires_confirmation: abc
)";

	auto result = xxlib::parser::parse_buffer(yaml);
	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "'requires_confirmation' must be a boolean (true/false)");
}

TEST(Parser_ParseBuffer, TemplateVarsAndRenderEngine) {
	const std::string yaml = R"(
alias:
  hello:
    - cmd: 'echo "{{ greeting }}, {{ target }}!"'
      render_engine: inja
      template_vars:
        greeting: Hello
        target:   World
)";

	auto result = xxlib::parser::parse_buffer(yaml);
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(result->size(), 1u);

	const Command& hello = result->at(0);
	EXPECT_EQ(hello.name, "hello");
	ASSERT_EQ(hello.cmd.size(), 1u);
	EXPECT_EQ(hello.cmd.at(0), R"(echo "{{ greeting }}, {{ target }}!")");
	EXPECT_EQ(hello.renderEngine, xxlib::renderer::Engine::Inja);
	EXPECT_EQ(hello.templateVars.at("greeting"), "Hello");
	EXPECT_EQ(hello.templateVars.at("target"), "World");
}

TEST(Parser_ParseBuffer, FlatListLayout) {
	const std::string yaml = R"(
aliases:
  - name: hello
    cmd: 'echo "{{ greeting }}, {{ target }}!"'
    render_engine: inja
    template_vars:
      greeting: Hello
      target:   World
)";

	auto result = xxlib::parser::parse_buffer(yaml);
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(result->size(), 1u);

	const Command& hello = result->at(0);
	EXPECT_EQ(hello.name, "hello");
	EXPECT_EQ(hello.renderEngine, xxlib::renderer::Engine::Inja);
	EXPECT_EQ(hello.templateVars.at("greeting"), "Hello");
	EXPECT_EQ(hello.templateVars.at("target"), "World");
}
