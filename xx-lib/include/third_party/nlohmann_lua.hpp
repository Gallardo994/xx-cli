#ifndef NLOHMANN_LUA_HPP
#define NLOHMANN_LUA_HPP

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace nlohmann::lua {
	[[nodiscard]] int json_parse(lua_State* L);
	[[nodiscard]] int json_dump(lua_State* L);
	[[nodiscard]] int luaopen_array(lua_State* L);
} // namespace nlohmann::lua

#endif // NLOHMANN_LUA_HPP
