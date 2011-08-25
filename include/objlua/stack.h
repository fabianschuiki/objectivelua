#pragma once
#include "describe.h"
#include "lua.h"


class LuaStack {
public:
	/** Serializes the stack and dumps it to the console. */
	static void dump(lua_State * L)
	{
		std::cout << "objlua: stack " << describe(L) << "\n";
	}
	
	/** Serializes the entire stack into a string. Very handy for debugging. */
	static std::string describe(lua_State * L)
	{
		std::stringstream s;
		
		//Find the number of items on the stack.
		int n = lua_gettop(L);
		s << "(\n";
		
		//Iterate through the stack from top to bottom and show each entry.
		for (int i = n; i > 0; i--) {
			//Extract the stack entry's type.
			int type = lua_type(L, i);
			
			//Dump the header.
			s << "    [" << i << "] ";
			
			//Dump the contents of whatever there is on the stack.
			s << LuaDescribe::generic(L, type, i, 1);
			
			s << "\n";
		}
		
		//Close the stack bracket.
		s << ")";
		return s.str();
	}
};
