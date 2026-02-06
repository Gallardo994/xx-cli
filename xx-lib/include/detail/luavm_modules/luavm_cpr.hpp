#ifndef XX_LUAVM_MODULES_CPR_HPP
#define XX_LUAVM_MODULES_CPR_HPP

#include <lua.hpp>

namespace xxlib::luavm::mod_cpr {
	[[nodiscard]] int luaopen_cpr(lua_State* L);
	[[nodiscard]] int cpr_get(lua_State* L);
	[[nodiscard]] int cpr_post(lua_State* L);
} // namespace xxlib::luavm::mod_cpr

#endif // XX_LUAVM_MODULES_CPR_HPP
