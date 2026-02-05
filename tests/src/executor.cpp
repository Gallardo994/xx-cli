#include "detail/command.hpp"
#include "detail/executor.hpp"
#include <gtest/gtest.h>

TEST(Executor_StringToExecutionEngine, System) {
	EXPECT_EQ(xxlib::executor::string_to_execution_engine("system"), xxlib::executor::Engine::System);
}

TEST(Executor_StringToExecutionEngine, Lua) {
	EXPECT_EQ(xxlib::executor::string_to_execution_engine("lua"), xxlib::executor::Engine::Lua);
}

TEST(Executor_StringToExecutionEngine, DotnetRun) {
    EXPECT_EQ(xxlib::executor::string_to_execution_engine("dotnet_run"), xxlib::executor::Engine::DotnetRun);
}

TEST(Executor_StringToExecutionEngine, Invalid) {
	EXPECT_THROW(auto _ = xxlib::executor::string_to_execution_engine("invalid"), std::invalid_argument);
}

TEST(Executor_ExecuteCommand, SystemEngine) {
	auto command = Command{
		.name = "test",
		.cmd = {"echo", "hello"},
		.executionEngine = xxlib::executor::Engine::System,
	};

	auto context = CommandContext{
		.dryRun = true,
	};

	auto result = xxlib::executor::execute_command(command, context);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result.value(), 0);
}

TEST(Executor_ExecuteCommand, LuaEngine) {
	auto command = Command{
		.name = "test",
		.cmd = {"return 0"},
		.executionEngine = xxlib::executor::Engine::Lua,
	};

	auto context = CommandContext{
		.dryRun = true,
	};

	auto result = xxlib::executor::execute_command(command, context);
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result.value(), 0);
}

TEST(Executor_ExecuteCommand, DotnetRunEngine) {
    auto command = Command{
        .name = "test",
        .cmd = {"using System; Console.WriteLine(6 * 7);"},
        .executionEngine = xxlib::executor::Engine::DotnetRun,
    };

    auto context = CommandContext{
        .dryRun = true,
    };

    auto result = xxlib::executor::execute_command(command, context);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0);
}

TEST(Executor_ExecuteCommand, UnknownEngine) {
	auto command = Command{
		.name = "test",
		.cmd = {"echo", "hello"},
		.executionEngine = static_cast<xxlib::executor::Engine>(42),
	};

	auto context = CommandContext{
		.dryRun = true,
	};

	auto result = xxlib::executor::execute_command(command, context);
	EXPECT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "Unknown execution engine");
}
