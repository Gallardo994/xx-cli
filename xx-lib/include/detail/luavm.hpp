#ifndef XX_LUAVM_HPP
#define XX_LUAVM_HPP

#include <memory>
#include <string>

struct lua_State;

namespace xxlib::luavm {
	struct Deleter {
		void operator()(lua_State* state) const;
	};

	[[nodiscard]] std::string version();

	using LuaStatePtr = std::unique_ptr<lua_State, Deleter>;

	[[nodiscard]] LuaStatePtr create();
	void destroy(LuaStatePtr& luaState);

	void add_json_library(LuaStatePtr& luaState);
	void add_cpr_library(LuaStatePtr& luaState);

	int32_t loadstring(LuaStatePtr& luaState, const std::string& code);
	int32_t pcall(LuaStatePtr& luaState, int32_t nargs, int32_t nresults, int32_t errfunc);

	[[nodiscard]] const char* tostring(LuaStatePtr& luaState, int32_t index = -1);
	[[nodiscard]] int64_t tointeger(LuaStatePtr& luaState, int32_t index = -1);
	[[nodiscard]] bool toboolean(LuaStatePtr& luaState, int32_t index = -1);

	[[nodiscard]] bool is_string(LuaStatePtr& luaState, int32_t index = -1);
	[[nodiscard]] bool is_integer(LuaStatePtr& luaState, int32_t index = -1);
	[[nodiscard]] bool is_boolean(LuaStatePtr& luaState, int32_t index = -1);
	[[nodiscard]] bool is_nil(LuaStatePtr& luaState, int32_t index = -1);

	void new_table(LuaStatePtr& luaState);
	const char* push_string(LuaStatePtr& luaState, const std::string& value);
	void push_boolean(LuaStatePtr& luaState, bool value);
	void set_table(LuaStatePtr& luaState, int32_t index);
	void set_global(LuaStatePtr& luaState, const std::string& name);
	void seti(LuaStatePtr& luaState, int32_t index, int64_t n);
} // namespace xxlib::luavm

#endif // XX_LUAVM_HPP
