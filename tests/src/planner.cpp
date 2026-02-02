#include "detail/planner.hpp"
#include "detail/platform.hpp"
#include <gtest/gtest.h>

#if _WIN32
const xxlib::platform::OS currentOs = xxlib::platform::OS::Windows;
const xxlib::platform::OSFamily currentOsFamily = xxlib::platform::OSFamily::Windows;
#elif __APPLE__
const xxlib::platform::OS currentOs = xxlib::platform::OS::MacOS;
const xxlib::platform::OSFamily currentOsFamily = xxlib::platform::OSFamily::Unix;
#elif __linux__
const xxlib::platform::OS currentOs = xxlib::platform::OS::Linux;
const xxlib::platform::OSFamily currentOsFamily = xxlib::platform::OSFamily::Unix;
#else
const xxlib::platform::OS currentOs = xxlib::platform::OS::Unknown;
const xxlib::platform::OSFamily currentOsFamily = xxlib::platform::OSFamily::Unknown;
#endif

#if __x86_64__ || _M_X64
const xxlib::platform::Architecture currentArchitecture = xxlib::platform::Architecture::x86_64;
#elif __aarch64__ || __arm64__
const xxlib::platform::Architecture currentArchitecture = xxlib::platform::Architecture::arm64;
#else
const xxlib::platform::Architecture currentArchitecture = xxlib::platform::Architecture::unknown;
#endif

TEST(Planner_MatchesConstraints, MatchesOS) {
	auto command = Command{
		.name = "test-command",
		.constraints = {{"os", xxlib::platform::os_to_string(currentOs)}},
	};
	EXPECT_TRUE(xxlib::planner::matches_constraints(command));
}

TEST(Planner_MatchesConstraints, MismatchesOS) {
	auto command = Command{
		.name = "test-command",
		.constraints = {{"os", "nonexistent_os"}},
	};
	EXPECT_FALSE(xxlib::planner::matches_constraints(command));
}

TEST(Planner_MatchesConstraints, MatchesArchitecture) {
	auto command = Command{
		.name = "test-command",
		.constraints = {{"arch", xxlib::platform::architecture_to_string(currentArchitecture)}},
	};
	EXPECT_TRUE(xxlib::planner::matches_constraints(command));
}

TEST(Planner_MatchesConstraints, MismatchesArchitecture) {
	auto command = Command{
		.name = "test-command",
		.constraints = {{"arch", "nonexistent_arch"}},
	};
	EXPECT_FALSE(xxlib::planner::matches_constraints(command));
}

TEST(Planner_MatchesConstraints, MatchesOSFamily) {
	auto command = Command{
		.name = "test-command",
		.constraints = {{"osfamily", xxlib::platform::os_family_to_string(currentOsFamily)}},
	};
	EXPECT_TRUE(xxlib::planner::matches_constraints(command));
}

TEST(Planner_MatchesConstraints, MismatchesOSFamily) {
	auto command = Command{
		.name = "test-command",
		.constraints = {{"osfamily", "nonexistent_osfamily"}},
	};
	EXPECT_FALSE(xxlib::planner::matches_constraints(command));
}

TEST(Planner_MatchesConstraints, MatchesMultipleConstraints) {
	auto command = Command{
		.name = "test-command",
		.constraints = {{"os", xxlib::platform::os_to_string(currentOs)}, {"arch", xxlib::platform::architecture_to_string(currentArchitecture)}},
	};
	EXPECT_TRUE(xxlib::planner::matches_constraints(command));
}

TEST(Planner_MatchesConstraints, MismatchesMultipleConstraints) {
	auto command = Command{
		.name = "test-command",
		.constraints = {{"os", xxlib::platform::os_to_string(currentOs)}, {"arch", "nonexistent_arch"}},
	};
	EXPECT_FALSE(xxlib::planner::matches_constraints(command));
}

TEST(Planner_PlanSingle, PlansSingleCommand) {
	auto commands = std::vector<Command>{{
		.name = "test-command",
		.constraints = {{"os", xxlib::platform::os_to_string(currentOs)}},
	}};
	auto result = xxlib::planner::plan_single(commands, "test-command");
	EXPECT_TRUE(result.has_value());
	EXPECT_EQ(result->name, "test-command");
}

TEST(Planner_PlanSingle, FailsOnNoMatchingCommand) {
	auto commands = std::vector<Command>{{
		.name = "test-command",
		.constraints = {{"os", "nonexistent_os"}},
	}};
	auto result = xxlib::planner::plan_single(commands, "test-command");
	EXPECT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "No matching command found for name: test-command");
}

TEST(Planner_PlanSingle, FailsOnMultipleMatchingCommands) {
	auto commands = std::vector<Command>{
		{.name = "test-command", .constraints = {{"os", xxlib::platform::os_to_string(currentOs)}}},
		{.name = "test-command", .constraints = {{"os", xxlib::platform::os_to_string(currentOs)}}},
	};
	auto result = xxlib::planner::plan_single(commands, "test-command");
	EXPECT_FALSE(result.has_value());
	EXPECT_EQ(result.error(), "Multiple matching commands found for name: test-command");
}
