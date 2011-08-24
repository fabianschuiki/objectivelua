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
        
        //Load the default libraries.
        luaL_openlibs(state);
    }
    /** Destructor which closes the state and cleans up. */
    ~LuaState() { lua_close(state); state = NULL; }
    
    /** Convenience cast operator so you can use the LuaState instance as if it
     were a normal lua_State. */
    operator lua_State * () { return state; }
    
    /** Convenience wrapper around the objlua_reportError function. */
    void reportError() { objlua_reportError(*this); }
    
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
		
		//Dump the stack.
		objlua_dumpStack(L);
        
        return 0;
    }
	
	/** Function exposed to Lua which dumps the current Lua stack to the
	 console. Great for debugging. */
	static int lua_dumpStack(lua_State * L) { objlua_dumpStack(L); return 0; }
};
