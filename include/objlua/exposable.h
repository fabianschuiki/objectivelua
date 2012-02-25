#pragma once
#include <cassert>
#include <cstdarg>
#include "error.h"
#include "lua.h"
#include "stack.h"
#include "state.h"


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
			{"new", T::lua_new},
			{"delete", T::lua_delete},
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
	
	/** Creates a new LuaExposable instance. The instance is not automatically constructed in Lua,
	 you have to do this manually by calling the constructLua function. */
	LuaExposable(lua_State * L) : L(L) {}
	
	/** Gets rid of the LuaExposable instance. */
	virtual ~LuaExposable()
	{
		//Remove the reference we hold to our instance.
		luaL_unref(L, LUA_REGISTRYINDEX, ref);
		ref = 0;
	}
	
	/** Constructs the Lua representation of this object. This function effectively instantiates
	 the class in Lua and links the instance to this C++ object.
	 
	 @param className	Name of the class to be instantiated. If NULL, the function takes the first
						value on the stack as a class table, i.e. will treat the stack as a function
						call to lua_new. */
	void constructLua(const char * className = NULL)
	{
		bool leaveOnStack = (className == NULL);
		
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
			if (argc < 1 || lua_type(L, 1) != LUA_TTABLE) {
				lua_pop(L, 1);
				luaL_error(L, "Trying to construct a LuaExposable "
						   "without a valid class table. Call Class:new() "
						   "instead of Class.new().\n");
				return;
			}
			lua_pushvalue(L, 1);
			lua_remove(L, 1);
		} else {
			lua_getglobal(L, className);
		}
		lua_getfield(L, -1, "__class");
		lua_insert(L, -2);
		lua_setmetatable(L, -3);
		
		//Try to get a grip on the constructor for this class.
		lua_gettable(L, -2);
		if (lua_isfunction(L, -1)) {
			//Move the function behin the first constructor argument.
			lua_insert(L, 1);
			
			//Duplicate the self, move one behind the function so it is available later, and one
			//after the function so it gets passed as the self argument.
			lua_pushvalue(L, -1);
			lua_insert(L, 1);
			lua_insert(L, 3);
			
			//Run the constructor.
			if (lua_pcall(L, (className ? argc + 1 : argc), 0, 0) != 0) {
				LuaState::stacktrace(L);
				LuaError::report(L);
			}
		} else {
			lua_pop(L, 1);
		}
		
		//Fetch a reference to the object. If we're required to leave the
		//initialized instance on the stack, we need to copy it so we may get
		//a reference.
		if (leaveOnStack)
			lua_pushvalue(L, -1);
		ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	
	/** Pushes this instance's table onto the Lua stack. */
	void loadReference() { lua_rawgeti(L, LUA_REGISTRYINDEX, ref); }
	
	/** Pushes the given function and this instance onto the Lua stack, so the
	 function may be called. */
	bool loadFunction(const char * fn)
	{
		assert(fn);
		
		//Resolve the reference and move it onto the stack.
		loadReference();
		
		//Get the requested function.
		lua_getfield(L, -1, fn);
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 2);
			//lua_pushfstring(L, "Unable to load unknown function \"%s\"", fn);
			return false;
		}
		
		//Move the function pointer behind the reference so the order of
		//arguments is correct for the Lua function calls.
		lua_insert(L, -2);
		return true;
	}
	
	/** Calls the Lua function with the given name. Each character in the format string specifies
	 the type of an argument to be passed to the function. The results field indicates how many re-
	 turn values are to be expected of the call.
	 
	 The following are the accepted argument types:
	 n  number (double)
	 s	C string (const char *)
	 o  Object (LuaExposable *)
	 */
	bool callFunction(const char * fn, const char * format = "", int results = 0, ...)
	{
		assert(fn && format);
		
		//Load the stacktrace.
		lua_getglobal(L, "stacktrace");
		int trace = lua_gettop(L);
		
		//Load the requested function.
		if (!loadFunction(fn)) {
			/*LuaState::stacktrace(L);
			LuaError::report(L);*/
			lua_remove(L, trace);
			return false;
		}
		
		//Fetch the argument list.
		va_list args;
		va_start(args, results);
		
		//Iterate through the format and push the argument to the Lua stack, depending on the format
		//specification.
		int argc = 0;
		for (const char * ptr = format; *ptr != 0; ptr++) {
			switch (*ptr) {
				case 'n':	lua_pushnumber(L, va_arg(args, double)); break;
				case 's':	lua_pushstring(L, va_arg(args, const char *)); break;
				case 'o':	va_arg(args, LuaExposable *)->loadReference(); break;
				default: {
					std::cerr << "objlua: *** Format error in call to function ";
					std::cerr << fn << ", type " << *ptr << " unknown\n";
					return false;
				} break;
			}
			argc++;
		}
		
		//Call the function.
		if (lua_pcall(L, argc + 1, results, trace) != 0) {
			LuaError::report(L);
			lua_remove(L, trace);
			return false;
		}
		
		//Get rid of the stacktrace global.
		lua_pop(L, 1);
		
		//We're done.
		va_end(args);
		return true;
	}
	
protected:
	/** Reference to the Lua state the instance of this object lives in. */
	lua_State * L;
	/** Reference to the Lua instance of this object. */
	int ref;
	
protected:
	/** Instantiates a new instance of the given class. */
	static int lua_new(lua_State * L)
	{
		T * instance = new T(L);
		instance->constructLua(NULL);
		return 1;
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

#define OBJLUA_CONSTRUCTOR(cls) cls(lua_State *L) : LuaExposable<cls>(L)
