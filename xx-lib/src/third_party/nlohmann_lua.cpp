#include <iostream>
#include "third_party/nlohmann_lua.hpp"

namespace nlohmann {
	const std::vector<struct luaL_Reg> lua::jsonLib_f = {
		{"new", lua::json_new},
		{"parse", lua::json_parse},
		{"type", lua::json_type},
		{nullptr, nullptr},
	};

	const std::vector<struct luaL_Reg> lua::jsonLib_m = {
		{"__index", lua::json_get},
		{"__newindex", lua::json_set},
		{"__len", lua::json_size},
		{"__tostring", lua::json_tostring},
		{"__gc", lua::json_gc},
		//{ "__pairs", lua::json_pairs },
		//{ "__ipairs", lua::json_ipairs },
		{nullptr, nullptr},
	};

	int lua::luaopen_array(lua_State* L) {
		luaL_newmetatable(L, MetadataTableName);

		lua_pushstring(L, "__index");
		lua_pushvalue(L, -2); /* pushes the metatable */
		lua_settable(L, -3);  /* metatable.__index = metatable */

		luaL_setfuncs(L, jsonLib_m.data(), 0);

		lua_newtable(L);
		luaL_setfuncs(L, jsonLib_f.data(), 0);
		lua_setglobal(L, "json");
		return 1;
	}

	int lua::json_new(lua_State* L) {
		new (lua_newuserdata(L, sizeof(nlohmann::json))) nlohmann::json();

		luaL_getmetatable(L, MetadataTableName);
		lua_setmetatable(L, -2);
		return 1;
	}

	int lua::json_parse(lua_State* L) {
		std::string jsonData = luaL_checkstring(L, 1);
		luaL_argcheck(L, !jsonData.empty(), 1, "`json_data` expected");

		nlohmann::json json;
		try {
			json = nlohmann::json::parse(jsonData);
		} catch (std::exception& ex) {
			luaL_argerror(L, 1, "Failed to parse JSON from `json_data`");
			return 0;
		}

		new (lua_newuserdata(L, sizeof(nlohmann::json))) nlohmann::json(std::move(json));

		luaL_getmetatable(L, MetadataTableName);
		lua_setmetatable(L, -2);

		return 1;
	}

	json& lua::getElement(lua_State* L, json& j, int index) {
		using vt = nlohmann::json::value_t;

		luaL_checkany(L, index);
		switch (lua_type(L, index)) {
		case LUA_TSTRING: {
			switch (j.type()) {
			default:
				luaL_argerror(L, 2, "Integer key is not supported");
				throw std::runtime_error("Unreachable, `luaL_argerror` never returns");
			case vt::object: {
				const char* key = luaL_checkstring(L, index);
				return j[key];
			}
			}
		}
		case LUA_TNUMBER: {
			switch (j.type()) {
			default:
				luaL_argerror(L, 2, "Integer key is not supported");
				throw std::runtime_error("Unreachable, `luaL_argerror` never returns");
			case vt::array: {
			    // Fixed in xx-lib, the original code uses 0-based indexing for Lua arrays which might be unintended.
				lua_Integer key = luaL_checkinteger(L, index);
				if (key < 1 || static_cast<std::size_t>(key) > j.size()) {
					luaL_argerror(L, index, "array index out of range");
				}
				return j[static_cast<std::size_t>(key) - 1];
			}
			case vt::object: {
				lua_Integer key = luaL_checkinteger(L, index);
				return j[std::to_string(key)];
			}
			}
		}
		default:
			luaL_argerror(L, index, "Unsupported key type");
			throw std::runtime_error("Unreachable, `luaL_argerror` never returns");
		}
	}

	int lua::push_json(lua_State* L, json& j) {
		using vt = nlohmann::json::value_t;

		switch (j.type()) {
		case vt::null: {
			lua_pushnil(L);
			return 1;
		}
		case vt::object: {
			new (lua_newuserdata(L, sizeof(nlohmann::json))) nlohmann::json(j);

			luaL_getmetatable(L, MetadataTableName);
			lua_setmetatable(L, -2);
			return 1;
		}
		case vt::array: {
			lua_newtable(L);
			for (int i = 0; i < j.size(); i++) {
				lua_pushinteger(L, j[i]);
				lua_rawseti(L, -2, i + 1);
			}
			return 1;
		}
		case vt::string: {
			lua_pushstring(L, j.get<std::string>().c_str());
			return 1;
		}
		case vt::boolean: {
			lua_pushboolean(L, j.get<bool>());
			return 1;
		}
		case vt::number_integer: {
			lua_pushinteger(L, j.get<int64_t>());
			return 1;
		}
		case vt::number_unsigned: {
			lua_pushinteger(L, j.get<uint64_t>());
			return 1;
		}
		case vt::number_float: {
			lua_pushnumber(L, j.get<double>());
			return 1;
		}
		case vt::binary:
		default:
			luaL_argerror(L, 2, "Unsupported JSON type");
			throw std::runtime_error("Unreachable, `luaL_argerror` never returns");
		}
	}

	int lua::json_get(lua_State* L) {
		json* j = checkjson(L, 1);
		json& value = getElement(L, *j, 2);
		return push_json(L, value);
	}

	int lua::json_set(lua_State* L) {
		json* j = checkjson(L, 1);
		json& value = getElement(L, *j, 2);

		luaL_checkany(L, 3);
		switch (lua_type(L, 3)) {
		case LUA_TNIL: {
			value.clear();
			return 0;
		}
		case LUA_TNUMBER: {
			double v = luaL_checknumber(L, 3);
			value = v;
			return 0;
		}
		case LUA_TBOOLEAN: {
			bool v = lua_toboolean(L, 3);
			value = v;
			return 0;
		}
		case LUA_TSTRING: {
			const char* v = luaL_checkstring(L, 3);
			value = v;
			return 0;
		}
		case LUA_TTABLE:
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TLIGHTUSERDATA:
		default: {
			luaL_argerror(L, 3, "Unsupported value type");
			throw std::runtime_error("Unreachable, `luaL_argerror` never returns");
		}
		}
	}

	int lua::json_size(lua_State* L) {
		json* j = checkjson(L, 1);

		lua_pushinteger(L, j->size());
		return 1;
	}

	int lua::json_tostring(lua_State* L) {
		json* j = checkjson(L, 1);

		lua_pushstring(L, j->dump().c_str());
		return 1;
	}

	int lua::json_gc(lua_State* L) {
		json* j = checkjson(L, 1);
		j->~json();
		return 0;
	}

	int lua::json_type(lua_State* L) {
		luaL_checkany(L, 1);

		void* ud = luaL_testudata(L, 1, MetadataTableName);
		if (ud == nullptr) {
			lua_pushstring(L, "non-json");
			return 1;
		}
		nlohmann::json* j = reinterpret_cast<nlohmann::json*>(ud);

		using vt = nlohmann::json::value_t;
		switch (j->type()) {
		case vt::null:
			lua_pushstring(L, "null");
			return 1;
		case vt::object:
			lua_pushstring(L, "object");
			return 1;
		case vt::array:
			lua_pushstring(L, "array");
			return 1;
		case vt::string:
			lua_pushstring(L, "string");
			return 1;
		case vt::boolean:
			lua_pushstring(L, "boolean");
			return 1;
		case vt::number_integer:
			lua_pushstring(L, "number_integer");
			return 1;
		case vt::number_unsigned:
			lua_pushstring(L, "number_unsigned");
			return 1;
		case vt::number_float:
			lua_pushstring(L, "number_float");
			return 1;
		case vt::binary:
			lua_pushstring(L, "binary");
			return 1;
		default:
		case vt::discarded:
			lua_pushstring(L, "unknown");
			return 1;
		}
	}

	/*
	static int luaB_next (lua_State *L)
	{
		lua::checkjson(L, 1);
		lua_settop(L, 2); // create a 2nd argument if there isn't one
		if (lua_next(L, 1))
		{
			return 2;
		}
		else
		{
			lua_pushnil(L);
			return 1;
		}
	}

	int lua::json_pairs(lua_State* L)
	{
		json* j = checkjson(L, 1);
		switch(j->type())
		{
			case nlohmann::json::value_t::array:
				return json_ipairs(L);
			case nlohmann::json::value_t::object:
			{
				luaL_checkany(L, 1);
				lua_pushcfunction(L, luaB_next); // will return generator
				lua_pushvalue(L, 1); // state
				lua_pushnil(L); // and initial value
				return 3;
			}
			default:
				return 0;
		}
	}

	static int ipairsaux (lua_State *L)
	{
		lua_Integer i = luaL_checkinteger(L, 2) + 1;
		lua_pushinteger(L, i);
		return (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
	}

	int lua::json_ipairs(lua_State* L)
	{
		json* j = checkjson(L, 1);
		if(j->type() != nlohmann::json::value_t::array)
			return 0;

		lua_pushcfunction(L, ipairsaux); // iteration function
		lua_pushvalue(L, 1); // state
		lua_pushinteger(L, 0); // initial value
		return 3;
	}
	*/
} // namespace nlohmann
