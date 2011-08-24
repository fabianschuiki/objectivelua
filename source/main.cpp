#include <iostream>
#include <objlua/objlua.h>

using namespace std;


int main(int argc, char * argv[])
{
    //Create a Lua state.
    LuaState lua;
    
    //Run a Lua file for debugging purposes.
    if (luaL_dofile(lua, "scripts/debug.lua"))
        lua.reportError();
    
    //Dump the stack.
    objlua_dumpStack(lua);
    
    return 0;
}
