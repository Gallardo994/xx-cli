#include "detail/helpers.hpp"
#include <gtest/gtest.h>

TEST(HelpersSplitExtras, SplitExtras) {
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
