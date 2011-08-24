#pragma once
#include <iostream>
#include <objlua/objlua.h>

using namespace std;


class Sprite {
public:
	static Sprite * fromStack(lua_State * L, int index)
	{
		//Check whether the type matches.
		luaL_checktype(L, index, LUA_TTABLE);
		
		//Get the userdata containing the pointer to the sprite instance.
		lua_getfield(L, index, "__this");
		if (!lua_isuserdata(L, -1)) {
			cerr << "objlua: *** __this is not userdata\n";
			return NULL;
		}
		void * s = lua_touserdata(L, -1);
		lua_pop(L, 1);
		
		//Return the pointer to the sprite.
		return *((Sprite **)s);
	}
	
	Sprite(lua_State * L, bool leaveOnStack = false) : L(L)
	{
		cout << "Sprite constructed\n";
		
		//Create a new table which will act as the sprite instance.
		lua_newtable(L);
		
		//Allocate memory for a pointer to the object.
		Sprite ** s = (Sprite **)lua_newuserdata(L, sizeof(Sprite *));
		*s = this;
		
		//Assign the class metatable for the userdata.
		luaL_getmetatable(L, "Class.Sprite");
		lua_setmetatable(L, -2);
		
		//Store the userdata under the __this index in the table.
		lua_setfield(L, -2, "__this");
		
		//Use the Sprite template table as this instance's metatable.
		lua_getglobal(L, "Sprite");
		lua_setmetatable(L, -2);
		
		//Fetch a reference to the sprite. If we're required to leave the
		//initialized instance on the stack, we need to copy it so we may get
		//a reference.
		if (leaveOnStack)
			lua_pushvalue(L, -1);
		ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	~Sprite()
	{
		cout << "Sprite destructed\n";
		
		//Remove the reference we hold to our instance.
		luaL_unref(L, LUA_REGISTRYINDEX, ref);
		ref = 0;
	}
	
	/** Pushes this instance's table onto the Lua stack. */
	void loadReference()
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	}
	
	/** Pushes the given function and this instance onto the Lua stack, so the
	 function may be called. */
	int loadFunction(const char * fn)
	{
		//Resolve the reference and move it onto the stack.
		loadReference();
		
		//Get the requested function.
		lua_getfield(L, -1, fn);
		if (!lua_isfunction(L, -1))
			return luaL_error(L, "Unable to load unknown function \"%s\"", fn);
		
		//Move the function pointer behind the reference so the order of
		//arguments is correct for the Lua function calls.
		lua_insert(L, -2);
		return 0;
	}
	
	static void expose(LuaState & L)
	{
		//Create the root metatable for this class.
		luaL_newmetatable(L, "Class.Sprite");
		luaL_register(L, NULL, lua_classFunctions);
		//luaL_register(L, NULL, lua_functions);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
		
		//Create a new global table for this class.
		luaL_register(L, "Sprite", lua_functions);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
		
		//Load the class script file.
		L.dofile("scripts/sprite.lua");
	}
	
	static const luaL_Reg lua_functions[];
	static const luaL_Reg lua_classFunctions[];
	
	static int lua_gc(lua_State * L)
	{
		/*cerr << "objlua: *** Object is being garbage collected. There is no"
		" function for handling this event!\n";*/
		return 0;
	}
	
	static int lua_say(lua_State * L)
	{
		cout << "Here I am, saying stuff.\n";
		return 0;
	}
	
	static int lua_new(lua_State * L)
	{
		new Sprite(L, true);
		return 1;
	}
	
	void animate()
	{
		//Push the animation function onto the stack.
		//lua_getglobal(L, "Sprite");
		//lua_getfield(L, -1, "animate");
		
		//Resolve the reference to the instance and push it onto the stack.
		loadFunction("animate");
		
		//Call the function.
		if (lua_pcall(L, 1, 0, 0) != 0)
			LuaError::report(L);
	}
	
	int ref;
	lua_State * L;
};
