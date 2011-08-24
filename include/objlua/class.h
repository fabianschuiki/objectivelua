#pragma once
#include <iostream>
#include "error.h"
#include "lua.h"
#include "stack.h"
#include "state.h"


class LuaClass {
public:
	/** Installs the class extensions in the given Lua state. **/
    static void install(lua_State * L)
	{
		static const luaL_Reg functions[] = {
			{"defineClass", lua_defineClass},
			{NULL, NULL}
		};
		
		//Create the global table for the class functions.
		luaL_register(L, "class", functions);
		
		//Create the metatable for the class table.
		lua_newtable(L);
		lua_getfield(L, -2, "defineClass");
		lua_setfield(L, -2, "__call");
		lua_setmetatable(L, -2);
		
		//Clean up after luaL_register which leaves the table on the stack.
		lua_pop(L, 1);
	}
	
	/** Creates a new bare class with the given name on the stack. If a super-
	 class index is provided, the new class will extend the given class. */
	static void make(lua_State * L, const char * className, int superclass = 0)
	{
		if (superclass < 0)
			superclass += lua_gettop(L);
		
		//Create the new class table.
		lua_newtable(L);
		
		//Since this table is going to be used as a metatable, make function
		//lookups occur in the metatable itself.
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		
		//Store the class name.
		lua_pushstring(L, className);
		lua_setfield(L, -2, "__class");
		
		//Set the superclass as the __index element. This will redirect un-
		//known function calls to the superclass.
		if (superclass) {
			lua_newtable(L);
			lua_pushvalue(L, superclass);
			lua_setfield(L, -2, "__index");
			lua_setmetatable(L, -2);
		}
		
		//Move the class table into the global namespace.
		lua_pushvalue(L, -1);
		lua_setglobal(L, className);
	}
	
	/** Same as make, but looks up the superclass by its name. */
	static void make(lua_State * L, const char * className,
					 const char * superclass)
	{
		lua_getglobal(L, superclass);
		make(L, className, -1);
		lua_pop(L, 1);
	}
	
private:
	/** Lua function to define a class. Takes the class name and optionally the
	 superclass table as arguments. Leaves nothing on the stack. */
	static int lua_defineClass(lua_State * L)
	{
		//Get the number of arguments.
		int argc = lua_gettop(L);
		if (argc < 2 || argc > 3)
			return luaL_error(L, "Expected 1-2 arguments, got %d", argc);
		
		//Extract the desired class name.
		const char * className = luaL_checkstring(L, 2);
		
		//Extract the superclass if there is one.
		int superclass = 0;
		if (argc >= 3) {
			luaL_checktype(L, 3, LUA_TTABLE);
			superclass = 3;
		}
		
		//Create the bare bones for this class.
		make(L, className, superclass);
		lua_pop(L, 1);
		return 0;
	}
};
