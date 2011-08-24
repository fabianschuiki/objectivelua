#pragma once
#include <iostream>
#include "lua.h"


/** Dumps an error to the console. Call this function after another call to the
 Lua API failed. */
void objlua_reportError(lua_State * L)
{
    const char * s = lua_tostring(L, -1);
    if (!s)
        s = "unknown Lua error";
    std::cerr << "objlua: *** " << s << "\n";
}
