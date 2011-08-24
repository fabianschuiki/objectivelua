#pragma once
#include "lua.h"
#include <sstream>
#include <string>

std::string objlua_describeGeneric(lua_State * L, int type, int index,
                                   int indent = 0);


/** Describes a string. */
std::string objlua_describeString(lua_State * L, int index)
{
    std::stringstream s;
    s << "\"";
    s << lua_tostring(L, index);
    s << "\"";
    return s.str();
}

/** Describes a number. */
std::string objlua_describeNumber(lua_State * L, int index)
{
    std::stringstream s;
    s << lua_tonumber(L, index);
    return s.str();
}

/** Describes a boolean. */
std::string objlua_describeBoolean(lua_State * L, int index)
{
	return (lua_toboolean(L, index) ? "true" : "false");
}

/** Describes a table. */
std::string objlua_describeTable(lua_State * L, int index,
                                 int indent = 0)
{
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
        s << objlua_describeGeneric(L, lua_type(L, -1), lua_gettop(L),
                                    indent + 1);
        s << "\n";
    }
    s << std::string(indent * 4, ' ') << "}";
    return s.str();
}


/** Calls the appropriate description function for the given type of item. */
std::string objlua_describeGeneric(lua_State * L, int type, int index,
                                   int indent)
{
    switch (type) {
        case LUA_TSTRING: return objlua_describeString(L, index);
        case LUA_TNUMBER: return objlua_describeNumber(L, index);
        case LUA_TBOOLEAN: return objlua_describeBoolean(L, index);
        case LUA_TTABLE:  return objlua_describeTable(L, index, indent);
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
