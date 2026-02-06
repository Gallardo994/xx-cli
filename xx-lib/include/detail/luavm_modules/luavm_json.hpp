#ifndef XX_LUAVM_MODULES_JSON_HPP
#define XX_LUAVM_MODULES_JSON_HPP

#include <lua.hpp>

namespace xxlib::luavm::mod_json {
	[[nodiscard]] int json_parse(lua_State* L);
	[[nodiscard]] int json_dump(lua_State* L);
	[[nodiscard]] int luaopen_array(lua_State* L);
} // namespace xxlib::luavm::mod_json

#endif // XX_LUAVM_MODULES_JSON_HPP
