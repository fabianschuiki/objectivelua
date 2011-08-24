#pragma once
#include "error.h"
#include "lua.h"


/** All the objects that are likely to be exposed to Lua should inherit from
 this superclass. This fasciliates function exposure and handling of the
 instance pairs between C++ and Lua. */
class LuaExposable {
public:
	/** Exposes a basic set of functions to Lua. Subclasses should have their
	 own expose function which first calls the parent's expose and then adds its
	 own functions to the instance. */
	template <typename T> static void expose(lua_State * L,
											 const char * className)
	{
		static const luaL_Reg functions[] = {
			{"new", lua_new<T>},
			{"delete", lua_delete},
			{NULL, NULL}
		};
		
		//Create the metatable for this class.
		luaL_register(L, className, functions);
	}
	
	/** Interprets the given stack item as an exposed object and tries to re-
	 trieve the C++ object pointer it is associated with.
	 
	 @return Returns NULL if retrieval was unsuccessful. */
	template <typename T> static T * fromStack(lua_State * L, int index)
	{
		//Check whether the type matches.
		luaL_checktype(L, index, LUA_TTABLE);
		
		//Extract the __this field which should contain the pointer.
		lua_getfield(L, index, "__this");
		
		//Check whether the userdata is valid and extract the pointer.
		if (!lua_isuserdata(L, -1)) {
			std::cerr << "objlua: *** Unable to retrieve C++ object from stack."
			" __this field does not contain userdata.\n";
			return NULL;
		}
		void * ptr = lua_touserdata(L, -1);
		lua_pop(L, 1);
		
		//Capture the unlikely event touserdata returns NULL.
		if (!ptr) {
			std::cerr << "objlua: *** C++ object retrieved from stack is"
			" NULL.\n";
			return NULL;
		}
		
		//Return the pointer to the exposable.
		return *((T **)ptr);
	}
	
	/** Creates a new LuaExposable instance and exposes it to the Lua state
	 passed. The given class name is used to lookup the Lua metatable that
	 contains the class inteface. If leaveOnStack is true, the resulting object
	 is left on the Lua stack so it may be used later on. */
	LuaExposable(lua_State * L, const char * className = NULL,
				 bool leaveOnStack = false) : L(L)
	{
		//Create a new table which will act as the object instance.
		lua_newtable(L);
		
		//Allocate memory for a pointer to the object.
		LuaExposable ** s = (LuaExposable **)lua_newuserdata(L, sizeof(LuaExposable *));
		*s = this;
		
		//Store the userdata under the __this index in the table.
		lua_setfield(L, -2, "__this");
		
		//Load the global table for this class and assign it as metatable. This
		//is either done by looking up the class table using the provided class
		//name, or using the first argument of the Lua function call on the
		//stack as table.
		if (!className) {
			luaL_checktype(L, 1, LUA_TTABLE);
			lua_pushvalue(L, 1);
		} else {
			lua_getglobal(L, className);
		}
		lua_setmetatable(L, -2);
		
		//Fetch a reference to the object. If we're required to leave the
		//initialized instance on the stack, we need to copy it so we may get
		//a reference.
		if (leaveOnStack)
			lua_pushvalue(L, -1);
		ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	
	/** Gets rid of the LuaExposable instance. */
	virtual ~LuaExposable()
	{
		//Remove the reference we hold to our instance.
		luaL_unref(L, LUA_REGISTRYINDEX, ref);
		ref = 0;
	}
	
	/** Pushes this instance's table onto the Lua stack. */
	void loadReference() { lua_rawgeti(L, LUA_REGISTRYINDEX, ref); }
	
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
	
private:
	/** Reference to the Lua state the instance of this object lives in. */
	lua_State * L;
	/** Reference to the Lua instance of this object. */
	int ref;
	
	/** Instantiates a new instance of the given class. */
	template <typename T> static int lua_new(lua_State * L)
	{
		new T(L, NULL); return 1;
	}
	
	/** Deletes the object. */
	static int lua_delete(lua_State * L)
	{
		//Get the pointer to the object.
		LuaExposable * obj = fromStack<LuaExposable>(L, -1);
		if (!obj)
			return luaL_error(L, "Unable to delete object.");
		
		//Delete the object.
		delete obj;
		return 0;
	}
};
