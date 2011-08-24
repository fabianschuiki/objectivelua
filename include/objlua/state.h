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
			reportError();
			return false;
		}
		return true;
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
		
		//Dump the stack.
		LuaStack::dump(L);
        
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
	
	static int lua_class(lua_State * L)
	{
	}
};
