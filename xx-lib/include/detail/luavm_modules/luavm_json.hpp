#ifndef XX_LUAVM__JSON_HPP
#define XX_LUAVM__JSON_HPP

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace xxlib::luavm::json {
	[[nodiscard]] int json_parse(lua_State* L);
	[[nodiscard]] int json_dump(lua_State* L);
	[[nodiscard]] int luaopen_array(lua_State* L);
} // namespace xxlib::luavm::json

#endif // XX_LUAVM__JSON_HPP
