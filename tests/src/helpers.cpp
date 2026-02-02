#include "detail/helpers.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <string>

TEST(Helpers_SplitExtras, SplitExtras) {
	auto extras = std::vector<std::string>{"key1=value1", "key2=value2", "positional1", "key3=value3", "positional2"};

	auto result = xxlib::helpers::split_extras(extras);

	EXPECT_EQ(result.kv.size(), 3);
	EXPECT_EQ(result.kv["key1"], "value1");
	EXPECT_EQ(result.kv["key2"], "value2");
	EXPECT_EQ(result.kv["key3"], "value3");

	EXPECT_EQ(result.positional.size(), 2);
	EXPECT_EQ(result.positional[0], "positional1");
	EXPECT_EQ(result.positional[1], "positional2");
}

TEST(Helpers_GetUnsetVars, GetUnsetVars) {
	auto templateVars = std::unordered_map<std::string, std::string>{
		{"var1", "value1"},
		{"var2", ""},
		{"var3", "value3"},
		{"var4", ""},
	};

	auto unsetVars = xxlib::helpers::get_uset_vars(templateVars);

	EXPECT_EQ(unsetVars.size(), 2);
	EXPECT_NE(std::find(unsetVars.begin(), unsetVars.end(), "var2"), unsetVars.end());
	EXPECT_NE(std::find(unsetVars.begin(), unsetVars.end(), "var4"), unsetVars.end());
}
