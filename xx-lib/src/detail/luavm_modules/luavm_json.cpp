#include "detail/luavm_modules/luavm_json.hpp"

#include <nlohmann/json.hpp>
#include <lua.h>

namespace xxlib::luavm::mod_json {
	const std::vector<struct luaL_Reg> functions = {
		{"parse", xxlib::luavm::mod_json::json_parse},
		{"dump", xxlib::luavm::mod_json::json_dump},
		{nullptr, nullptr},
	};

	int json_to_lua(lua_State* L, nlohmann::json& j) {
		using vt = nlohmann::json::value_t;

		switch (j.type()) {
		case vt::null: {
			lua_pushnil(L);
			return 1;
		}
		case vt::object: {
			lua_newtable(L);
			for (auto& el : j.items()) {
				lua_pushstring(L, el.key().c_str());
				json_to_lua(L, el.value());
				lua_settable(L, -3);
			}
			return 1;
		}
		case vt::array: {
			lua_newtable(L);
			for (int i = 0; i < j.size(); i++) {
				json_to_lua(L, j[i]);
				lua_seti(L, -2, i + 1);
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
			throw std::runtime_error("Unreachable, `luaL_argerror` never returns");
		}
	}

	nlohmann::json lua_to_json(lua_State* L, int idx) {
		const auto type = lua_type(L, idx);

		switch (type) {
		case LUA_TNIL:
			return nlohmann::json(nullptr);
		case LUA_TBOOLEAN: {
			auto b = lua_toboolean(L, idx) != 0;
			return nlohmann::json(b);
		}
		case LUA_TNUMBER: {
			auto i = lua_tointeger(L, idx);
			if (lua_isinteger(L, idx)) {
				return nlohmann::json(static_cast<int64_t>(i));
			} else {
				auto d = lua_tonumber(L, idx);
				return nlohmann::json(static_cast<double>(d));
			}
		}
		case LUA_TSTRING: {
			size_t len;
			const auto* s = lua_tolstring(L, idx, &len);
			return nlohmann::json(std::string(s, len));
		}
		case LUA_TTABLE: {
			lua_pushnil(L);

			if (lua_next(L, idx) == 0) {
				return nlohmann::json::object();
			}

			nlohmann::json j;
			auto is_array = true;

			do {
				if (is_array && lua_type(L, -2) == LUA_TNUMBER) {
					auto key = lua_tointeger(L, -2);
					if (key != j.size() + 1) {
						is_array = false;
					}
				} else {
					is_array = false;
				}

				auto value = lua_to_json(L, lua_gettop(L));
				lua_pop(L, 1);

				if (!is_array) {
					size_t key_len;
					const auto* key_str = lua_tolstring(L, -1, &key_len);
					j[std::string(key_str, key_len)] = value;
				} else {
					j.push_back(value);
				}
			} while (lua_next(L, idx) != 0);

			return j;
		}
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TLIGHTUSERDATA:
		default:
			throw std::runtime_error("Unsupported Lua type for JSON conversion");
		}
	}

	int luaopen_array(lua_State* L) {
		lua_newtable(L);
		luaL_setfuncs(L, functions.data(), 0);
		lua_setglobal(L, "json");
		return 1;
	}

	int json_parse(lua_State* L) {
		auto* jsonData = luaL_checkstring(L, 1);
		if (!jsonData) {
			luaL_argerror(L, 1, "`json_data` must be a string");
			return 0;
		}

		try {
			auto json = nlohmann::json::parse(jsonData);
			json_to_lua(L, json);
			return 1;
		} catch (std::exception& ex) {
			luaL_argerror(L, 1, "Failed to parse JSON from `json_data`");
		}

		return 0;
	}

	int json_dump(lua_State* L) {
		try {
			auto j = lua_to_json(L, 1);
			auto minify = lua_toboolean(L, 2) != 0;

			std::string dumped;
			if (minify) {
				dumped = j.dump();
			} else {
				dumped = j.dump(4);
			}
			lua_pushstring(L, dumped.c_str());
			return 1;
		} catch (const std::exception& ex) {
			luaL_error(L, "Failed to dump JSON: %s", ex.what());
			return 0;
		}
	}
} // namespace xxlib::luavm::mod_json
