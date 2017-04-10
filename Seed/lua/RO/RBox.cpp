#include "RBox.h"
#include "..\DD\DrawMachine.h"
#include "Windows.h"

RBox::RBox(unsigned short int classId) : RenderObject(classId) {
}

void RBox::__init(lua_State *L) {

	RenderObject::__init(L);

	auto c = lua_gettop(L);
	// check params	
	if (c < 7) {
		luaL_where(L, 1);
		lua_pushfstring(L, "%RText need 7 arguments but have %d", lua_tostring(L, -1), getCount());
		lua_remove(L, -2);
		lua_error(L);
	}

	_helper_set(L, 1, 4, ROF_X);
	_helper_set(L, 5, 7, RBF_COLOR);

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

RBox::~RBox() {

}

/*
void RRect::checkArgument(lua_State *L, ArrayItem *item, int position) {


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

*/


void RBox::__drawSelf(DDDrawMachine *drawMachine) {

	auto p = getData();

	auto _x = p[ROF_X].i;
	auto _y = p[ROF_Y].i;

	auto _w = p[ROF_W].i;
	auto _h = p[ROF_H].i;

	auto _s = p[RBF_SIZE].i;
	auto color1 = p[RBF_COLOR].i;
	auto color2 = p[RBF_RCOLOR].i;


	while (_s--) {
		drawMachine->__drawRect((int)_x, (int)_y, (int)_w, 1, (int)color2);
		drawMachine->__drawRect((int)_x, (int)(_y + _h - 1), (int)_w, 1, (int)color2);
		drawMachine->__drawRect((int)_x, (int)_y + 1, 1, (int)(_h - 2), (int)color2);
		drawMachine->__drawRect((int)(_x + _w - 1), (int)_y + 1, 1, (int)(_h - 2), (int)color2);
		_x++;
		_y++;
		_w -= 2;
		_h -= 2;
	}
	drawMachine->__drawRect((int)_x, (int)_y, (int)_w, (int)_h, (int)color1);

}


LuaAMethods(RBox, [](lua_State *L, int methods) { LuaAddMethods(RenderObject); }) = {
	// LuaMethodDesc(RRect, get),
	{ 0,0 }
};

LuaMetaMethods(RBox) = {
	// LuaMethodDesc(Array, __tostring),
	// LuaMethodDesc(Font, __newindex),
	{ 0,0 }
};


void module_rbox(lua_State *L) {
	LuaClass<RBox>::Register(L);
}
