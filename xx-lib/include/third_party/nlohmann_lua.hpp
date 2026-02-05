#ifndef NLOHMANN_LUA_HPP
#define NLOHMANN_LUA_HPP

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "third_party/json.hpp"

namespace nlohmann {
	class lua {
	  public:
		lua() = delete;

		static const std::vector<struct luaL_Reg> jsonLib_f;

		[[nodiscard]] static int json_parse(lua_State* L);
		[[nodiscard]] static int json_dump(lua_State* L);

		static int luaopen_array(lua_State* L);

	  private:
		static int json_to_lua(lua_State* L, nlohmann::json& j);
		static nlohmann::json lua_to_json(lua_State* L, int idx);
	};
} // namespace nlohmann

#endif // NLOHMANN_LUA_HPP
