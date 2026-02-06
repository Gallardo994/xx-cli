#include "detail/luavm_modules/luavm_cpr.hpp"

#include <cpr/cpr.h>
#include <lua.h>
#include <lauxlib.h>

namespace xxlib::luavm::mod_cpr {
	const std::vector<struct luaL_Reg> functions = {
		{"get", cpr_get},
		{"post", cpr_post},
		{nullptr, nullptr},
	};

	static void push_response(lua_State* L, const cpr::Response& r) {
		lua_newtable(L);

		lua_pushstring(L, "status_code");
		lua_pushinteger(L, r.status_code);
		lua_settable(L, -3);

		lua_pushstring(L, "body");
		lua_pushstring(L, r.text.c_str());
		lua_settable(L, -3);

		lua_pushstring(L, "headers");
		lua_newtable(L);
		for (const auto& [k, v] : r.header) {
			lua_pushstring(L, k.c_str());
			lua_pushstring(L, v.c_str());
			lua_settable(L, -3);
		}
		lua_settable(L, -3);

		lua_pushstring(L, "url");
		lua_pushstring(L, r.url.str().c_str());
		lua_settable(L, -3);

		lua_pushstring(L, "error");
		if (r.error) {
			lua_pushstring(L, r.error.message.c_str());
		} else {
			lua_pushnil(L);
		}
		lua_settable(L, -3);
	}

	static cpr::Parameters extract_params(lua_State* L, int start_arg = 2) {
		cpr::Parameters params{};
		if (lua_isnoneornil(L, start_arg))
			return params;

		if (!lua_istable(L, start_arg)) {
			return params;
		}

		lua_pushnil(L);

		while (lua_next(L, start_arg) != 0) {
			if (lua_isstring(L, -2) && lua_isstring(L, -1)) {
				const auto* key = lua_tostring(L, -2);
				const auto* value = lua_tostring(L, -1);
				params.Add({key, value});
			}

			lua_pop(L, 1);
		}

		return params;
	}

	static cpr::Header extract_headers(lua_State* L, int idx) {
		cpr::Header header{};
		if (!lua_istable(L, idx)) {
			return header;
		}

		lua_pushnil(L);

		while (lua_next(L, idx) != 0) {
			if (lua_isstring(L, -2) && lua_isstring(L, -1)) {
				const auto* key = lua_tostring(L, -2);
				const auto* value = lua_tostring(L, -1);
				header[key] = value;
			}

			lua_pop(L, 1);
		}

		return header;
	}

	int cpr_get(lua_State* L) {
		const auto* url = luaL_checkstring(L, 1);

		cpr::Parameters params{};
		if (lua_gettop(L) >= 2) {
			params = extract_params(L, 2);
		}

		cpr::Header headers{};
		if (lua_gettop(L) >= 3) {
			headers = extract_headers(L, 3);
		}

		try {
			auto r = cpr::Get(cpr::Url{url}, params, headers);
			push_response(L, r);
			return 1;
		} catch (const std::exception& ex) {
			lua_newtable(L);
			lua_pushstring(L, "error");
			lua_pushstring(L, ex.what());
			lua_settable(L, -3);
			return 1;
		}
	}

	int cpr_post(lua_State* L) {
		const auto* url = luaL_checkstring(L, 1);

		cpr::Body body{};
		if (lua_gettop(L) >= 2 && lua_isstring(L, 2)) {
			body = lua_tostring(L, 2);
		}

		cpr::Parameters params{};
		if (lua_gettop(L) >= 3) {
			params = extract_params(L, 3);
		}

		cpr::Header headers{};
		if (lua_gettop(L) >= 4) {
			headers = extract_headers(L, 4);
		}

		try {
			auto r = cpr::Post(cpr::Url{url}, body, params, headers);
			push_response(L, r);
			return 1;
		} catch (const std::exception& ex) {
			lua_newtable(L);
			lua_pushstring(L, "error");
			lua_pushstring(L, ex.what());
			lua_settable(L, -3);
			return 1;
		}
	}

	int luaopen_cpr(lua_State* L) {
		lua_newtable(L);
		luaL_setfuncs(L, functions.data(), 0);
		lua_setglobal(L, "cpr");
		return 1;
	}

} // namespace xxlib::luavm::mod_cpr
