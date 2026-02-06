#include "detail/luavm.hpp"
#include "detail/luavm_modules/luavm_json.hpp"
#include "detail/luavm_modules/luavm_cpr.hpp"

#include <lua.hpp>

namespace xxlib::luavm {
	void Deleter::operator()(lua_State* state) const {
		lua_close(state);
	}

	LuaStatePtr create() {
		lua_State* state = luaL_newstate();
		if (!state) {
			return nullptr;
		}
		luaL_openlibs(state);
		return LuaStatePtr(state);
	}

	void destroy(LuaStatePtr& luaState) {
		lua_close(luaState.get());
	}

	void add_json_library(LuaStatePtr& luaState) {
		auto _ = mod_json::luaopen_array(luaState.get());
	}

	void add_cpr_library(LuaStatePtr& luaState) {
        auto _ = mod_cpr::luaopen_cpr(luaState.get());
    }

	std::string version() {
		return LUA_RELEASE;
	}

	int32_t loadstring(LuaStatePtr& luaState, const std::string& code) {
		return luaL_loadstring(luaState.get(), code.c_str());
	}

	int32_t pcall(LuaStatePtr& luaState, int32_t nargs, int32_t nresults, int32_t errfunc) {
		return lua_pcall(luaState.get(), nargs, nresults, errfunc);
	}

	int64_t tointeger(LuaStatePtr& luaState, int32_t index) {
		return static_cast<int64_t>(lua_tointeger(luaState.get(), index));
	}

	const char* tostring(LuaStatePtr& luaState, int32_t index) {
		return lua_tostring(luaState.get(), index);
	}

	bool toboolean(LuaStatePtr& luaState, int32_t index) {
		return lua_toboolean(luaState.get(), index) != 0;
	}

	bool is_integer(LuaStatePtr& luaState, int32_t index) {
		return lua_isinteger(luaState.get(), index) != 0;
	}

	bool is_string(LuaStatePtr& luaState, int32_t index) {
		return lua_isstring(luaState.get(), index) != 0;
	}

	bool is_boolean(LuaStatePtr& luaState, int32_t index) {
		return lua_isboolean(luaState.get(), index) != 0;
	}

	bool is_nil(LuaStatePtr& luaState, int32_t index) {
		return lua_isnil(luaState.get(), index) != 0;
	}

	void new_table(LuaStatePtr& luaState) {
		lua_newtable(luaState.get());
	}

	const char* push_string(LuaStatePtr& luaState, const std::string& value) {
		return lua_pushstring(luaState.get(), value.c_str());
	}

	void push_boolean(LuaStatePtr& luaState, bool value) {
		lua_pushboolean(luaState.get(), value ? 1 : 0);
	}

	void set_table(LuaStatePtr& luaState, int32_t index) {
		lua_settable(luaState.get(), index);
	}

	void set_global(LuaStatePtr& luaState, const std::string& name) {
		lua_setglobal(luaState.get(), name.c_str());
	}

	void seti(LuaStatePtr& luaState, int32_t index, int64_t n) {
		lua_seti(luaState.get(), index, static_cast<lua_Integer>(n));
	}
} // namespace xxlib::luavm
