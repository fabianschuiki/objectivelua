#pragma once
#include "lua.h"
#include <sstream>
#include <string>


class LuaDescribe {
public:
	/** Describes a string. */
	static std::string string(lua_State * L, int index)
	{
		std::stringstream s;
		s << "\"";
		s << lua_tostring(L, index);
		s << "\"";
		return s.str();
	}
	
	/** Describes a number. */
	static std::string number(lua_State * L, int index)
	{
		std::stringstream s;
		s << lua_tonumber(L, index);
		return s.str();
	}
	
	/** Describes a boolean. */
	static std::string boolean(lua_State * L, int index)
	{
		return (lua_toboolean(L, index) ? "true" : "false");
	}
	
	/** Describes a table. */
	static std::string table(lua_State * L, int index,
							 int indent = 0)
	{
		if (index < 0) index += lua_gettop(L) + 1;
		std::stringstream s;
		std::string pad((indent + 1) * 4, ' ');
		s << "{\n";
		for (lua_pushnil(L); lua_next(L, index); lua_pop(L, 1)) {
			
			//Copy the key to the top of the stack, since luaL_checkstring messes with it which would
			//break lua_next if it did so in-place.
			lua_pushvalue(L, -2);
			const char * key = lua_tostring(L, -1);
			lua_pop(L, 1);
			
			//Add the entry description.
			s << pad << key << " = ";
			if (std::string(key) == "__index")
				s << "...";
			else
				s << generic(L, lua_type(L, -1), -1,
							 indent + 1);
			s << "\n";
		}
		s << std::string(indent * 4, ' ') << "}";
		return s.str();
	}
	
	
	/** Calls the appropriate description function for the given type of item. */
	static std::string generic(lua_State * L, int type, int index,
							   int indent = 0)
	{
		switch (type) {
			case LUA_TSTRING: return string(L, index);
			case LUA_TNUMBER: return number(L, index);
			case LUA_TBOOLEAN: return boolean(L, index);
			case LUA_TTABLE:  return table(L, index, indent);
		}
		
		//Fallback if we're unable to describe the given type.
		std::stringstream s;
		s << "<";
		s << lua_typename(L, type);
		s << " @";
		s << index;
		s << ">";
		return s.str();
	}
};
