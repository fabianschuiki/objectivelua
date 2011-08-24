#include "sprite.h"


const luaL_Reg Sprite::lua_functions[] = {
	{"new", Sprite::lua_new},
	{"say", Sprite::lua_say},
	{NULL, NULL}
};
const luaL_Reg Sprite::lua_classFunctions[] = {
	{"__gc", Sprite::lua_gc},
	{NULL, NULL}
};
