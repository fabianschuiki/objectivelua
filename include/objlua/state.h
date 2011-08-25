#pragma once
#include "error.h"
#include "lua.h"
#include "stack.h"


class LuaState {
public:
    /** Constructor which initializes the state. */
    LuaState()
    {
        //Open a new lua state.
        state = lua_open();
        if (!state) {
            std::cerr << "objlua: *** unable to open new lua state\n";
            return;
        }
        
        //Register the basic panic fallback.
        lua_atpanic(state, lua_panic);
		
		//Register helper functions.
		lua_register(state, "dumpStack", lua_dumpStack);
		lua_register(state, "dump", lua_dump);
        
        //Load the default libraries.
        luaL_openlibs(state);
		
		//Reset the stack so we get a clean working area.
		lua_settop(state, 0);
		
		//Register the stacktrace function which may be used as an error function for Lua errors.
		lua_register(state, "stacktrace", stacktrace);
    }
    /** Destructor which closes the state and cleans up. */
    ~LuaState() { lua_close(state); state = NULL; }
    
    /** Convenience cast operator so you can use the LuaState instance as if it
     were a normal lua_State. */
    operator lua_State * () { return state; }
    
    /** Convenience wrapper around the objlua_reportError function. */
    void reportError() { LuaError::report(*this); }
	
	/** Convenience wrapper around luaL_dofile which automatically reports any
	 errors that might occur. */
	bool dofile(const char * fn)
	{
		if (luaL_dofile(state, fn)) {
			stacktrace(state);
			reportError();
			return false;
		}
		return true;
	}
    
	/** Takes the error at the top of the stack and converts it to a string, then appends a trace-
	 back for debugging purposes. */
	static int stacktrace(lua_State * L)
	{
		//Convert the error to a string.
		lua_getglobal(L, "tostring");
		lua_insert(L, -2);
		lua_call(L, 1, 1);
		
		//Call Lua's traceback function. It will take the error message and append the trace. Since
		//we don't want ourself to appear in the trace, we start at level 2.
		lua_getglobal(L, "debug");
		lua_getfield(L, -1, "traceback");
		lua_pushvalue(L, -3);
		lua_pushinteger(L, 2);
		lua_call(L, 2, 1);
		lua_insert(L, -3);
		
		//Move the result behind the debug global and the initial error message so they can both be
		//popped off the stack.
		lua_pop(L, 2);
		
		return 1;
	}
	
private:
    /** The wrapped lua state. **/
    lua_State * state;
    
    /** Basic panic function which functions as a last resort for Lua panics
     that aren't caught by regular code. */
    static int lua_panic(lua_State * L)
    {
        //Dump a general warning.
        std::cerr << "objlua: *** PANIC: ";
        std::cerr.flush();
        
        //Show the error details.
        if (lua_type(L, -1) == LUA_TSTRING)
            std::cerr << lua_tostring(L, -1) << "\n";
        else
            std::cerr << lua_topointer(L, -1) << "\n";
        std::cerr.flush();
		lua_pop(L, 1);
        
        return 0;
    }
	
	/** Function exposed to Lua which dumps the current Lua stack to the
	 console. Great for debugging. */
	static int lua_dumpStack(lua_State * L) { LuaStack::dump(L); return 0; }
	
	/** Function exposed to Lua which dumps the given parameter to the
	 console. */
	static int lua_dump(lua_State * L)
	{
		int top = lua_gettop(L);
		std::cout << LuaDescribe::generic(L, lua_type(L, top), top) << "\n";
		return 0;
	}
};
