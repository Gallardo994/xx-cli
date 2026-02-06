#ifndef XX_LUAVM_MODULES_FS_HPP
#define XX_LUAVM_MODULES_FS_HPP

#include <lua.hpp>

namespace xxlib::luavm::mod_fs {
	[[nodiscard]] int luaopen_fs(lua_State* L);
} // namespace xxlib::luavm::mod_fs

#endif // XX_LUAVM_MODULES_FS_HPP
