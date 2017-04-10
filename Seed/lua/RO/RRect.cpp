#include "RRect.h"
#include "..\DD\DrawMachine.h"

RRect::RRect(unsigned short int classId) : RenderObject(classId) {
}

void RRect::__init(lua_State *L) {

	RenderObject::__init(L);

	auto c = lua_gettop(L);
	// check params
	if (c < 5) {
		luaL_where(L, 1);
		lua_pushfstring(L, "%sRRect need 5 arguments but have %d", lua_tostring(L, -1), getCount());
		lua_remove(L, -2);
		lua_error(L);
	}

	_helper_set(L, 1, 4, ROF_X);
	_helper_set(L, 5, 5, RRF_COLOR);

	auto p = getData();
	p[ROF_BX].type = UDT_Integer;
	p[ROF_BX].i = p[ROF_X].i;

	p[ROF_BY].type = UDT_Integer;
	p[ROF_BY].i = p[ROF_Y].i;


	/*
	auto p = getData();
	for (int i = 0; i < 5; i++, p++) {
		checkArgument(L, p, i);
	}
	*/
}

RRect::~RRect() {

}

void RRect::checkArgument(lua_State *L, ArrayItem *item, int position) {

	/*
	char b[1024];
	sprintf_s(b, "RRect::checkArgument %d", position);
	OutputDebugStringA(b);
	*/

	switch (position) {
	case ROF_X:
	case ROF_Y:
	case ROF_W:
	case ROF_H:
	case RRF_COLOR:
		if (item->type != UDT_Integer) {
			luaL_where(L, 1);
			lua_pushfstring(L, "%sRRect argument %d wrong type '%s'", lua_tostring(L, -1), position + 1, udtypename(item->type));
			lua_remove(L, -2);
			lua_error(L);
		}
	}

}


void RRect::__drawSelf(DDDrawMachine *drawMachine) {

	auto p = getData();

	auto _x = p[ROF_X].i;
	auto _y = p[ROF_Y].i;
	auto _w = p[ROF_W].i;
	auto _h = p[ROF_H].i;
	auto color = p[RRF_COLOR].i;

	drawMachine->__drawRect((int)_x, (int)_y, (int)_w, (int)_h, (int)color);
}


LuaAMethods(RRect, [](lua_State *L, int methods) { LuaAddMethods(RenderObject); }) = {
	// LuaMethodDesc(RRect, get),
	{ 0,0 }
};

LuaMetaMethods(RRect) = {
	// LuaMethodDesc(Array, __tostring),
	// LuaMethodDesc(Font, __newindex),
	{ 0,0 }
};


void module_rrect(lua_State *L) {
	LuaClass<RRect>::Register(L);
}
