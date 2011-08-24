#include <iostream>
#include <objlua/objlua.h>
#include "sprite.h"

using namespace std;


int main(int argc, char * argv[])
{
    //Create a Lua state.
    LuaState lua;
	LuaClass::install(lua);
    
	//Expose the Sprite class.
	Sprite::expose(lua);
	
    //Run a Lua file for debugging purposes.
	lua.dofile("scripts/debug.lua");
    
	//Now the script should actually have returned a Sprite instance.
	/*Sprite * sprite = Sprite::fromStack<Sprite>(lua, -1);
	sprite->animate();*/
    
    return 0;
}
