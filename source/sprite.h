#pragma once
#include <iostream>
#include <objlua/objlua.h>

using namespace std;


class Sprite {
public:
	Sprite() { cout << "Sprite constructed\n"; }
	~Sprite() { cout << "Sprite destructed\n"; }
	
	static Sprite * lastInstance;
	
	static void expose(LuaState & L)
	{
		cout << "exposing\n";
		
		/*luaL_newmetatable(L, "Dude.Sprite");
		luaL_register(L, NULL, luaFunctions);*/
		//lua_pushvalue(L, -1);
		//lua_setfield(L, -2, "__index");
		
		//Create a new global table for this class.
		luaL_register(L, "Sprite", luaFunctions);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
		
		//Load the class script file.
		L.dofile("scripts/sprite.lua");
	}
	
	static const luaL_Reg luaFunctions[];
	
	static int lua_say(lua_State * L)
	{
		cout << "<Sprite> asked to say something\n";
	}
	
	static int lua_new(lua_State * L)
	{
		//Get the number of arguments.
		int n = lua_gettop(L);
		
		//Create a new table which will act as the sprite instance.
		lua_newtable(L);
		
		//Allocate memory for a pointer to the object and stored it under the
		//"this" index.
		Sprite ** s = (Sprite **)lua_newuserdata(L, sizeof(Sprite *));
		lua_setfield(L, -2, "__this");
		
		//Initialize the sprite to this pointer.
		lastInstance = new Sprite();
		*s = lastInstance;
		
		//Use the Sprite template table as this instance's metatable.
		lua_getglobal(L, "Sprite");
		lua_setmetatable(L, -2);
		
		//The finished sprite is on top of the stack.
		lua_pushvalue(L, -1);
		(*s)->ref = luaL_ref(L, LUA_REGISTRYINDEX);
		return 1;
	}
	
	int ref;
	void animate(LuaState & L)
	{
		//Push the animation function onto the stack.
		lua_getglobal(L, "Sprite");
		lua_getfield(L, -1, "animate");
		
		//Resolve the reference to the instance and push it onto the stack.
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
		
		//Call the function.
		if (lua_pcall(L, 1, 0, 0) != 0) {
			L.reportError();
		}
		
		//Get rid of the global table that held the function.
		lua_pop(L, 1);
	}
};
