#pragma once

#include "lua.h"

#define CHECK_integer(n) (lua_isnumber(L, n) == 0)
#define CHECK_string(n) (lua_isstring(L, n) == 0)
#define CHECK_function(n) (lua_isfunction(L, n) == 0)
#define CHECK_array(n) (! (lua_isuserdata(L, n) && LC_checkUserDataClass(L, n, "Array")) )
#define CHECK_window(n) (! (lua_isuserdata(L, n) && LC_checkUserDataClass(L, n, "Window")) )

#define CHECK_ARG(n, t) if( CHECK_##t(n) ) { luaL_typerror(L, n, #t); }
#define CHECK_ARGCLASS(n, t, v) if( ! (lua_isuserdata(L, n) && LC_checkUserDataClass(L, n, #t)) ) { luaL_typerror(L, n, #t); } auto ud##n = (UserData *)lua_touserdata(L, n); auto v = (t *)ud##n->data;
#define CHECK_OPTARG(n, t) if( cs >= n && CHECK_##t(n) ) { luaL_typerror(L, n, #t); }


bool LC_checkUserDataClass(lua_State *L, int idx, const char *className);
