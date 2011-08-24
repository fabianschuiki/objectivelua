#include "sprite.h"

Sprite * Sprite::lastInstance = NULL;

const luaL_Reg Sprite::luaFunctions[] = {
	{"new", Sprite::lua_new},
	{"say", Sprite::lua_say},
	{NULL, NULL}
};
