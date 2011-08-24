#pragma once

#include "class.h"
#include "describe.h"
#include "error.h"
#include "exposable.h"
#include "lua.h"
#include "stack.h"
#include "state.h"


/*!
 @mainpage ObjectiveLua Introduction
 
 
 @section Overview
 
 ObjectiveLua is a lightweight, header-only library that allows for easy and
 low-level bridging between C++ and Lua. The project's aim is to provide a basic
 mechanism for classes and class inheritance in Lua scripts, as well as means to
 call C++ functions from Lua and vice versa.
 
 
 @section Basic Lua Helpers
 
 The library provides a few helper classes that wrap services and functionality
 associated with Lua.
 
 @subsection State
 @code
 //Creating an instance of LuaState automatically initializes a new Lua state
 //with the default libraries preloaded and some debugging functions registered.
 LuaState lua;
 
 //The following executes a script from a file, reporting potential errors.
 lua.dofile("myscript.lua");
 @endcode
 
 @subsection Errors
 @code
 //Reports and pops the error on top of the stack of the Lua state L.
 LuaError::report(L);
 @endcode
 
 @subsection Structure Descriptions
 The LuaDescribe closure provides functions that allow you to convert Lua data
 structures to human-readable strings.
 @code
 //The following will serialize the topmost element on the Lua stack and write
 //the string representation to the console.
 std::cout << LuaDescribe::generic(L, -1) << std::endl;
 @endcode
 Also available in Lua:
 @code
 dump(myVar)
 @endcode
 
 @subsection Stack
 The following dumps the entire Lua stack to the console. This function is
 pretty handy for debugging your Lua-exposed objects.
 @code
 LuaStack::dump(L);
 @endcode
 Also available in Lua:
 @code
 dumpStack()
 @endcode
 
 
 @section a Class Mechanism
 
 The LuaClass closure provides functions that allow for a basic class
 inheritance system in Lua. In order to use it, you first have to register the
 required functions in your Lua state:
 @code
 LuaState lua;
 LuaClass::install(lua);
 @endcode
 
 With this installed, you may create new classes from C++:
 @code
 LuaClass::make(lua, "MyClass")
 @endcode
 Essentially, classes in Lua are global tables named after their class.
 
 If you want to subclass another class, you can do this either by passing the
 superclass's name, or if you already have it on the stack, the its stack index.
 @code
 LuaClass::make(lua, "MyClass", "MySuperclass");
 LuaClass::make(lua, "MyClass", 5);
 @endcode
 
 You may also create classes in Lua directly:
 @code
 class("MyClass", MySuperclass)
 
 function MyClass:helloWorld
	print("...")
 end
 @endcode
 Note that you have to give the name of your new class as a string, but the
 superclass table directly.
 */
