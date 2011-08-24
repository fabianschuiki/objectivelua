#pragma once
#include "describe.h"
#include "lua.h"


/** Dumps the entire stack of the given Lua virtual machine. */
void objlua_dumpStack(lua_State * L)
{
    //Find the number of items on the stack.
    int n = lua_gettop(L);
    std::cout << "objlua: stack (\n";
    
    //Iterate through the stack from top to bottom and show each entry.
    for (int i = n; i > 0; i--) {
        //Extract the stack entry's type.
        int type = lua_type(L, i);
        
        //Dump the header.
        std::cout << "    [" << i << "] ";
        
        //Dump the contents of whatever there is on the stack.
        std::cout << objlua_describeGeneric(L, type, i, 1);
        
        std::cout << "\n";
    }
    
    //Close the stack bracket.
    std::cout << ")\n";
}
