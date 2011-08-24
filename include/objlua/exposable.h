#pragma once
#include "error.h"
#include "lua.h"
#include "stack.h"


/** All the objects that are likely to be exposed to Lua should inherit from
 this superclass. This fasciliates function exposure and handling of the
 instance pairs between C++ and Lua. */
template <typename T> class LuaExposable {
public:
	/** Exposes a basic set of functions to Lua. Classes which would like to ex-
	 pose to Lua should first create their own class table using LuaClass::make,
	 call LuaExposable::expose to add the basic funcitonality and register their
	 own functions.*/
	static void expose(lua_State * L)
	{
		static const luaL_Reg functions[] = {
			{"new", lua_new},
			{"delete", lua_delete},
			{NULL, NULL}
		};
		luaL_register(L, 0, functions);
	}
	
	/** Interprets the given stack item as an exposed object and tries to re-
	 trieve the C++ object pointer it is associated with.
	 
	 @return Returns NULL if retrieval was unsuccessful. */
	static T * fromStack(lua_State * L, int index)
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
		//Count the arguments supplied to the Lua function.
		int argc = lua_gettop(L);
		
		//Create a new table which will act as the object instance.
		lua_newtable(L);
		
		
		//Allocate memory for a pointer to the object.
		LuaExposable ** s = (LuaExposable **)lua_newuserdata(L, sizeof(LuaExposable<T> *));
		
		//Now to some magic. We need the pointer to point at the entire class, not only at the
		//LuaExposable portion. The following snippet of code shifts the this pointer appropriately.
		//It is taken from the "Enginuity Part II" tutorial on gamedev.net.
		long offset = (long)(T *)1 - (long)(LuaExposable<T> *)(T *)1;
		*s = (LuaExposable<T> *)((long)this + offset);
		
		//Store the userdata under the __this index in the table.
		lua_setfield(L, -2, "__this");
		
		//Load the global table for this class and assign it as metatable. This
		//is either done by looking up the class table using the provided class
		//name, or using the first argument of the Lua function call on the
		//stack as table.
		if (!className) {
			if (argc != 1 || lua_type(L, 1) != LUA_TTABLE) {
				lua_pop(L, 1);
				luaL_error(L, "Trying to construct a LuaExposable "
						   "without a valid class table. Call Class:new() "
						   "instead of Class.new().\n");
				return;
			}
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
	
	/** Convenience function that calls a Lua method, i.e. a function without
	 any arguments and return values. Returns whether the call was successful.*/
	int callMethod(const char * fn)
	{
		//Load the function to be called.
		int err = loadFunction(fn);
		if (err != 0)
			return err;
		
		//Call the function.
		err = lua_pcall(L, 1, 0, 0);
		if (err != 0)
			LuaError::report(L);
		return err;
	}
	
protected:
	/** Reference to the Lua state the instance of this object lives in. */
	lua_State * L;
	/** Reference to the Lua instance of this object. */
	int ref;
	
private:
	/** Instantiates a new instance of the given class. */
	static int lua_new(lua_State * L)
	{
		new T(L, NULL, true); return 1;
	}
	
	/** Deletes the object. */
	static int lua_delete(lua_State * L)
	{
		//Get the pointer to the object.
		T * obj = fromStack(L, -1);
		if (!obj)
			return luaL_error(L, "Unable to delete object.");
		
		//Delete the object.
		delete obj;
		return 0;
	}
};


/** Creates a wrapper method for a certain Lua method. Convenience. */
#define OBJLUA_WRAP_METHOD(func) void func() { callMethod(#func); }

/** Synthesizes the default constructor for the given class and a default class name. */
#define OBJLUA_CONSTRUCTOR_WITH_CLASS_NAME(cls, name) \
cls(lua_State * L, const char * className = name, bool leaveOnStack = false) \
: LuaExposable<cls>(L, className, leaveOnStack)

/** Synthesizes the default constructor for the given class. */
#define OBJLUA_CONSTRUCTOR(cls) OBJLUA_CONSTRUCTOR_WITH_CLASS_NAME(cls, #cls)
