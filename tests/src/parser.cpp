#include "detail/parser.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

std::string create_test_file(const std::string& name, const std::string& content) {
	auto tempFilePath = std::filesystem::temp_directory_path() / name;
	std::ofstream outFile(tempFilePath);
	outFile << content;
	outFile.close();
	return tempFilePath;
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

TEST(Parser_ParseBuffer, BasicTomlSingleCommand) {
	std::string str = R"(
[[alias.build]]
cmd = "echo Building project"
)";

	auto result = xxlib::parser::parse_buffer(str);
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(result->size(), 1);

	auto& buildCommand = result->at(0);
	EXPECT_EQ(buildCommand.name, "build");
	EXPECT_EQ(buildCommand.cmd.size(), 1);
	EXPECT_EQ(buildCommand.cmd.at(0), "echo Building project");
}

TEST(Parser_ParseBuffer, BasicTomlArray) {
	std::string str = R"(
[[alias.cmd1]]
cmd = "echo Building project"

[[alias.cmd2]]
cmd = "echo Running tests"
)";

	auto result = xxlib::parser::parse_buffer(str);
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(result->size(), 2);

	auto& cmd1 = result->at(0);
	EXPECT_EQ(cmd1.name, "cmd1");
	EXPECT_EQ(cmd1.cmd.size(), 1);
	EXPECT_EQ(cmd1.cmd.at(0), "echo Building project");

	auto& cmd2 = result->at(1);
	EXPECT_EQ(cmd2.name, "cmd2");
	EXPECT_EQ(cmd2.cmd.size(), 1);
	EXPECT_EQ(cmd2.cmd.at(0), "echo Running tests");
}

TEST(Parser_ParseBuffer, TomlWithConstraints) {
	std::string str = R"(
[[alias.build]]
cmd = "echo Building project"
constraints = [ [ 'os', 'linux' ], [ 'arch', 'x86_64' ] ]

[[alias.test]]
cmd = "echo Running tests"
constraints = [ [ 'osfamily', 'windows' ] ]
)";

	auto result = xxlib::parser::parse_buffer(str);
	ASSERT_TRUE(result.has_value());
	EXPECT_EQ(result->size(), 2);

	auto& buildCommand = result->at(0);
	EXPECT_EQ(buildCommand.name, "build");
	EXPECT_EQ(buildCommand.cmd.size(), 1);
	EXPECT_EQ(buildCommand.cmd.at(0), "echo Building project");
	EXPECT_EQ(buildCommand.constraints.size(), 2);
	EXPECT_EQ(buildCommand.constraints.at(0).first, "os");
	EXPECT_EQ(buildCommand.constraints.at(0).second, "linux");
	EXPECT_EQ(buildCommand.constraints.at(1).first, "arch");
	EXPECT_EQ(buildCommand.constraints.at(1).second, "x86_64");

	auto& testCommand = result->at(1);
	EXPECT_EQ(testCommand.name, "test");
	EXPECT_EQ(testCommand.cmd.size(), 1);
	EXPECT_EQ(testCommand.cmd.at(0), "echo Running tests");
	EXPECT_EQ(testCommand.constraints.size(), 1);
	EXPECT_EQ(testCommand.constraints.at(0).first, "osfamily");
	EXPECT_EQ(testCommand.constraints.at(0).second, "windows");
}

TEST(Parser_ParseBuffer, InvalidToml) {
	std::string str = R"(
[[alias.invalid]]
cmd = "echo I forgot to close the array"
constraints = [ [ 'os', 'linux'
)";

	auto result = xxlib::parser::parse_buffer(str);
	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "TOML parse error: Error while parsing array: encountered end-of-file");
}

TEST(Parser_ParseBuffer, MissingCmdField) {
	std::string str = R"(
[[alias.empty_cmd]]
)";

	auto result = xxlib::parser::parse_buffer(str);
	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "Missing cmd field");
}

TEST(Parser_ParseBuffer, InvalidConstraintType) {
	std::string str = R"(
[[alias.invalid_constraint]]
cmd = "echo Testing invalid constraint"
constraints = [ "not_a_pair" ]
)";
	auto result = xxlib::parser::parse_buffer(str);
	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "Invalid constraint item type or size");
}

TEST(Parser_ParseBuffer, InvalidRequiresConfirmationType) {
	std::string str = R"(
[[alias.invalid_confirmation]]
cmd = "echo Testing invalid confirmation"
requires_confirmation = "yes"
)";
	auto result = xxlib::parser::parse_buffer(str);
	ASSERT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "Invalid requires_confirmation type");
}
