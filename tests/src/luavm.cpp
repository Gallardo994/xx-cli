#include "detail/luavm.hpp"
#include <gtest/gtest.h>

TEST(LuaVM_Version, ReturnsVersionString) {
	auto version = xxlib::luavm::version();
	EXPECT_EQ(version, "Lua 5.4.7");
}

TEST(LuaVM_CreateDestroy, CreatesAndDestroysLuaState) {
	auto luaState = xxlib::luavm::create();
	EXPECT_NE(luaState, nullptr);
}

TEST(LuaVM_LoadString, LoadsValidLuaCode) {
	auto luaState = xxlib::luavm::create();

	auto loadStatus = xxlib::luavm::loadstring(luaState, "return 42");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);
}

TEST(LuaVM_LoadString, FailsOnInvalidLuaCode) {
	auto luaState = xxlib::luavm::create();

	auto loadStatus = xxlib::luavm::loadstring(luaState, "return }");
	EXPECT_NE(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_NE(pcallStatus, 0);
}

TEST(LuaVM_Types, PushAndRetrieveTypes) {
	auto luaState = xxlib::luavm::create();

	xxlib::luavm::push_string(luaState, "Hello, Lua!");
	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), "Hello, Lua!");
	xxlib::luavm::set_global(luaState, "greeting");

	xxlib::luavm::push_boolean(luaState, true);
	EXPECT_TRUE(xxlib::luavm::is_boolean(luaState));
	EXPECT_TRUE(xxlib::luavm::toboolean(luaState));
	xxlib::luavm::set_global(luaState, "flag");
}

TEST(LuaVM_Table, CreateAndSetTable) {
	auto luaState = xxlib::luavm::create();

	xxlib::luavm::new_table(luaState);
	xxlib::luavm::push_string(luaState, "key1");
	xxlib::luavm::push_string(luaState, "value1");
	xxlib::luavm::set_table(luaState, -3);

	xxlib::luavm::push_string(luaState, "key2");
	xxlib::luavm::push_string(luaState, "value2");
	xxlib::luavm::set_table(luaState, -3);

	xxlib::luavm::set_global(luaState, "myTable");

	auto loadStatus = xxlib::luavm::loadstring(luaState, "return myTable.key1, myTable.key2");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 2, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState, -2));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState, -2)), "value1");

	EXPECT_TRUE(xxlib::luavm::is_string(luaState, -1));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState, -1)), "value2");
}

TEST(LuaVM_JsonParse, ParsesJsonString) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('{"key": "value", "number": 123}')
        return j.key, j.number
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 2, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState, -2));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState, -2)), "value");

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -1));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -1), 123);
}

TEST(LuaVM_JsonParse, ParsesNestedJsonString) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('{"outer": {"inner": "deep value"}}')
        return j.outer.inner
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), "deep value");
}

TEST(LuaVM_JsonParse, ArrayParsing) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('["first", "second", "third"]')
        return #j, j[1], j[2], j[3]
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 4, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -4));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -4), 3);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState, -3));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState, -3)), "first");

	EXPECT_TRUE(xxlib::luavm::is_string(luaState, -2));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState, -2)), "second");

	EXPECT_TRUE(xxlib::luavm::is_string(luaState, -1));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState, -1)), "third");
}

TEST(LuaVM_JsonDump, ConvertsJsonToString) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
		local j = json.parse('{"key": "value", "number": 123}')
		return json.dump(j, true)
	)");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), R"({"key":"value","number":123})");
}

TEST(LuaVM_JsonDump, ParseDumpEmptyJsonObject) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('{}')
        return json.dump(j, true)
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), R"({})");
}

TEST(LuaVM_JsonDump, ParseDumpEmptyJsonArray) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('[]')
        return json.dump(j, true)
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), R"({})");
}

TEST(LuaVM_JsonDump, ConvertsNestedJsonToString) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
		local j = json.parse('{"outer": {"inner": "deep value"}}')
		return json.dump(j, true)
	)");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), R"({"outer":{"inner":"deep value"}})");
}

TEST(LuaVM_JsonDump, ConvertsArrayJsonToString) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
		local j = json.parse('["first", "second", "third"]')
		return json.dump(j, true)
	)");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), R"(["first","second","third"])");
}

TEST(LuaVM_JsonDump, ModifyAndDumpJson) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
		local j = json.parse('{"key": "value", "number1": 123, "number2": 456.78}')
		j.key = "new value"
		j.number1 = 456
		j.number2 = 910.11
		return json.dump(j, true)
	)");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), R"({"key":"new value","number1":456,"number2":910.11})");
}

TEST(LuaVM_JsonDump, DumpBooleanAndNullValues) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('{"a": true, "b": false, "c": null}')
        return json.dump(j, true)
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), R"({"a":true,"b":false})");
}

TEST(LuaVM_JsonDump, UnminifiedOutput) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('{"key": "value", "number": 123}')
        return json.dump(j, false)
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 1, 0);
	EXPECT_EQ(pcallStatus, 0);

	EXPECT_TRUE(xxlib::luavm::is_string(luaState));
	const auto expectedOutput = R"({
    "key": "value",
    "number": 123
})";
	EXPECT_EQ(std::string(xxlib::luavm::tostring(luaState)), expectedOutput);
}

TEST(LuaVM_JsonPairs, PairsForObject) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('{"a": 1, "b": 2, "c": 3}')
        local result = {}
        for k, v in pairs(j) do
            result[k] = v
        end
        return result.a, result.b, result.c
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 3, 0);
	EXPECT_EQ(pcallStatus, 0);
	if (pcallStatus != 0) {
		const auto* errorMsg = xxlib::luavm::tostring(luaState, -1);
		FAIL() << "Lua error: " << (errorMsg ? errorMsg : "Unknown error");
	}

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -3));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -3), 1);

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -2));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -2), 2);

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -1));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -1), 3);
}

TEST(LuaVM_JsonPairs, PairsForArray) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('[10, 20, 30]')
        local result = {}
        for k, v in pairs(j) do
            result[k] = v
        end
        return result[1], result[2], result[3]
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 3, 0);
	EXPECT_EQ(pcallStatus, 0);
	if (pcallStatus != 0) {
		const auto* errorMsg = xxlib::luavm::tostring(luaState, -1);
		FAIL() << "Lua error: " << (errorMsg ? errorMsg : "Unknown error");
	}

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -3));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -3), 10);

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -2));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -2), 20);

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -1));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -1), 30);
}

TEST(LuaVM_JsonIpairs, IpairsForArray) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('[10, 20, 30]')
        local result = {}

        print(type(j))

        for i, v in ipairs(j) do
            print(tostring(i) .. ": " .. tostring(v))
            result[i] = v
        end
        return result[1], result[2], result[3]
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 3, 0);
	EXPECT_EQ(pcallStatus, 0);
	if (pcallStatus != 0) {
		const auto* errorMsg = xxlib::luavm::tostring(luaState, -1);
		FAIL() << "Lua error: " << (errorMsg ? errorMsg : "Unknown error");
	}

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -3));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -3), 10);

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -2));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -2), 20);

	EXPECT_TRUE(xxlib::luavm::is_integer(luaState, -1));
	EXPECT_EQ(xxlib::luavm::tointeger(luaState, -1), 30);
}

TEST(LuaVM_JsonIpairs, IpairsOnlyForArrays) {
	auto luaState = xxlib::luavm::create();
	xxlib::luavm::add_json_library(luaState);

	auto loadStatus = xxlib::luavm::loadstring(luaState, R"(
        local j = json.parse('{"a": 1}')
        for i, v in ipairs(j) do
            return i, v
        end
    )");
	EXPECT_EQ(loadStatus, 0);

	auto pcallStatus = xxlib::luavm::pcall(luaState, 0, 2, 0);
	EXPECT_EQ(pcallStatus, 0);
	EXPECT_TRUE(xxlib::luavm::is_nil(luaState, -2));
}
