#include <iostream>
#include <objlua/objlua.h>
#include "sprite.h"

using namespace std;


int main(int argc, char * argv[])
{
    //Create a Lua state.
    LuaState lua;
    
	//Expose the Sprite class.
	Sprite::expose(lua);
	
    //Run a Lua file for debugging purposes.
    if (luaL_dofile(lua, "scripts/debug.lua"))
        lua.reportError();
    
	std::cout << "--- will call animate() now\n";
	
	//Call the animate function of one of the sprites.
	Sprite::lastInstance->animate(lua);
    
    return 0;
}
