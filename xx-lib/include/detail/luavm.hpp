#ifndef XX_LUAVM_HPP
#define XX_LUAVM_HPP

#include <memory>
#include <string>

struct lua_State;

namespace xxlib {
	namespace luavm {
		struct Deleter {
			void operator()(lua_State* state) const;
		};

		std::string version();

		using LuaStatePtr = std::unique_ptr<lua_State, Deleter>;

		LuaStatePtr create();
		int32_t loadstring(LuaStatePtr& luaState, const std::string& code);
		int32_t pcall(LuaStatePtr& luaState, int32_t nargs, int32_t nresults, int32_t errfunc);

		const char* tostring(LuaStatePtr& luaState, int32_t index = -1);
		int64_t tointeger(LuaStatePtr& luaState, int32_t index = -1);
		bool toboolean(LuaStatePtr& luaState, int32_t index = -1);

		bool is_string(LuaStatePtr& luaState, int32_t index = -1);
		bool is_integer(LuaStatePtr& luaState, int32_t index = -1);
		bool is_boolean(LuaStatePtr& luaState, int32_t index = -1);
		bool is_nil(LuaStatePtr& luaState, int32_t index = -1);

		void new_table(LuaStatePtr& luaState);
		const char* push_string(LuaStatePtr& luaState, const std::string& value);
		void push_boolean(LuaStatePtr& luaState, bool value);
		void set_table(LuaStatePtr& luaState, int32_t index);
		void set_global(LuaStatePtr& luaState, const std::string& name);
		void seti(LuaStatePtr& luaState, int32_t index, int64_t n);
	} // namespace luavm
} // namespace xxlib

#endif // XX_LUAVM_HPP
