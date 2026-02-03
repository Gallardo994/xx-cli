#include "detail/command.hpp"
#include <gtest/gtest.h>

TEST(Command_JoinCmd, SingleCommand) {
	Command command;
	command.cmd = {"echo hello"};

	EXPECT_EQ(xxlib::command::join_cmd(command), "echo hello");
}

TEST(Command_JoinCmd, CommandArray) {
	Command command;
	command.cmd = {"echo", "hello", "world"};

	EXPECT_EQ(xxlib::command::join_cmd(command), "echo hello world");
}

TEST(Command_JoinConstraints, Empty) {
	Command command;
	command.constraints = {};

	EXPECT_EQ(xxlib::command::join_constraints(command), "");
}

TEST(Command_JoinConstraints, Single) {
	Command command;
	command.constraints = {{"os", "linux"}};

	EXPECT_EQ(xxlib::command::join_constraints(command), "os=linux");
}

TEST(Command_JoinConstraints, Multiple) {
	Command command;
	command.constraints = {{"os", "linux"}, {"arch", "x86_64"}};

	EXPECT_EQ(xxlib::command::join_constraints(command), "os=linux, arch=x86_64");
}
