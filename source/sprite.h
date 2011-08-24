#pragma once
#include <iostream>
#include <objlua/objlua.h>

using namespace std;


class Sprite : public LuaExposable {
public:
	Sprite(lua_State * L, const char * className, bool leaveOnStack = false)
	: LuaExposable(L, className, leaveOnStack) {}
	
	static void expose(LuaState & L)
	{
		//Create the new class.
		LuaClass::make(L, "Sprite");
		
		//Expose the base functions.
		LuaExposable::expose<Sprite>(L);
		
		//Register functions.
		static const luaL_Reg functions[] = {
			{"say", Sprite::lua_say},
			{NULL, NULL}
		};
		luaL_register(L, 0, functions);
		lua_pop(L, 1);
		
		//Load the class script file.
		L.dofile("scripts/sprite.lua");
	}
	
	static int lua_say(lua_State * L)
	{
		cout << "Here I am, saying stuff.\n";
		return 0;
	}
	
	LuaExposable_WrapCallMethod(animate)
};
