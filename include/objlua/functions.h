#pragma once
#include <cassert>
#include <cstdarg>
#include "lua.h"

class Lua
{
public:
	/** Pushes the given reference's table onto the Lua stack. */
	static void loadReference(lua_State * L, int ref)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	}
	
	/** Pushes the given function and the instance pointed to by ref onto the Lua stack, so the
	 function may be called. */
	static bool loadFunction(lua_State * L, const char * fn, int ref)
	{
		assert(fn);
		
		//Resolve the reference and move it onto the stack.
		loadReference(L, ref);
		
		//Get the requested function.
		lua_getfield(L, -1, fn);
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 2);
			//lua_pushfstring(L, "Unable to load unknown function \"%s\"", fn);
			return false;
		}
		
		//Move the function pointer behind the reference so the order of
		//arguments is correct for the Lua function calls.
		lua_insert(L, -2);
		return true;
	}
	
	static bool callFunctionProlog(lua_State * L, const char * fn, int ref, int & trace)
	{
		assert(fn);
		
		//Load the stacktrace.
		lua_getglobal(L, "stacktrace");
		trace = lua_gettop(L);
		
		//Load the requested function.
		if (!loadFunction(L, fn, ref)) {
			LuaState::stacktrace(L);
			LuaError::report(L);
			lua_remove(L, trace);
			return false;
		}
		
		return true;
	}
	
	static bool callFunctionEpilog(lua_State * L, const char * fn, int ref, int trace, int argc, int results = 0)
	{
		//Call the function.
		if (lua_pcall(L, argc + 1, results, trace) != 0) {
			LuaError::report(L);
			lua_remove(L, trace);
			return false;
		}
		
		//Get rid of the stacktrace global.
		lua_remove(L, trace);
		
		//We're done.
		return true;
	}
	
	static bool callFunction(lua_State * L, const char * fn, int ref, int results = 0, const char * format = "", ...)
	{
		assert(fn && format);
		int trace;
		
		if (!callFunctionProlog(L, fn, ref, trace)) {
			return false;
		}
		
		//Iterate through the format and push the argument to the Lua stack, depending on the format
		//specification.
		va_list args;
		va_start(args, format);
		int argc = 0;
		for (const char * ptr = format; *ptr != 0; ptr++) {
			switch (*ptr) {
				case 'n':	lua_pushnumber(L, va_arg(args, double)); break;
				case 's':	lua_pushstring(L, va_arg(args, const char *)); break;
				case 'o':	loadReference(L, va_arg(args, int)); break;
				default: {
					std::cerr << "objlua: *** Format error in call to function ";
					std::cerr << fn << ", type " << *ptr << " unknown\n";
					return false;
				} break;
			}
			argc++;
		}
		va_end(args);
		
		return callFunctionEpilog(L, fn, ref, trace, argc, results);
	}
};
