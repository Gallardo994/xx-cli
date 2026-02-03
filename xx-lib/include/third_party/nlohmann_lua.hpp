#ifndef NLOHMANN_LUA_HPP
#define NLOHMANN_LUA_HPP

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <memory>
#include "third_party/json.hpp"

namespace nlohmann {
	class lua {
	  public:
		lua() = delete;

	  public:
		static constexpr const char* MetadataTableName = "json";

	  public:
		[[nodiscard]] static int json_new(lua_State* L);
		[[nodiscard]] static int json_parse(lua_State* L);

		[[nodiscard]] static int json_type(lua_State* L);

	  public:
		static int luaopen_array(lua_State* L);
		static nlohmann::json* checkjson(lua_State* L, int index = 1) {
			return reinterpret_cast<nlohmann::json*>(luaL_checkudata(L, index, MetadataTableName));
		}
		static int push_json(lua_State* L, nlohmann::json& j);
		static json& getElement(lua_State* L, json& j, int index);

	  public:
		[[nodiscard]] static int json_get(lua_State* L);
		[[nodiscard]] static int json_set(lua_State* L);
		[[nodiscard]] static int json_size(lua_State* L);
		[[nodiscard]] static int json_tostring(lua_State* L);
		[[nodiscard]] static int json_gc(lua_State* L);
		//[[nodiscard]] static int json_pairs(lua_State* L);
		//[[nodiscard]] static int json_ipairs(lua_State* L);

	  public:
		static const std::vector<struct luaL_Reg> jsonLib_f;
		static const std::vector<struct luaL_Reg> jsonLib_m;
	};
} // namespace nlohmann

#endif // NLOHMANN_LUA_HPP
