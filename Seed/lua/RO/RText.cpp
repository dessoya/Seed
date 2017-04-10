#include "RText.h"
#include "..\DD\DrawMachine.h"
#include "..\Windows.h"

RText::RText(unsigned short int classId) : RenderObject(classId) {
}

void RText::__init(lua_State *L) {

	RenderObject::__init(L);

	auto c = lua_gettop(L);
	// check params	
	if (c < 5) {
		luaL_where(L, 1);
		lua_pushfstring(L, "%RText need 5 arguments but have %d", lua_tostring(L, -1), getCount());
		lua_remove(L, -2);
		lua_error(L);
	}

	_helper_set(L, 1, 2, ROF_X);
	_helper_set(L, 3, 5, RTF_TEXT);

	auto p = getData();

	p[ROF_W].type = UDT_Integer;
	p[ROF_W].i = 0;

	p[ROF_H].type = UDT_Integer;
	p[ROF_H].i = 0;

	p[ROF_SKIPHOVER].b = true;

	// auto p = getData();
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

RText::~RText() {

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


void RText::__drawSelf(DDDrawMachine *drawMachine) {

	auto p = getData();

	auto _x = p[ROF_X].i;
	auto _y = p[ROF_Y].i;

	auto _s = p[RTF_TEXT].str;
	auto bc = p[RTF_FONT].baseClass;
	auto font = dynamic_cast<Font *>(bc);
	auto color = p[RTF_COLOR].i;

	drawMachine->__drawText((int)_x, (int)_y, _s, font->_font, (int)color);
}


LuaAMethods(RText, [](lua_State *L, int methods) { LuaAddMethods(RenderObject); }) = {
	// LuaMethodDesc(RRect, get),
	{ 0,0 }
};

LuaMetaMethods(RText) = {
	// LuaMethodDesc(Array, __tostring),
	// LuaMethodDesc(Font, __newindex),
	{ 0,0 }
};


void module_rtext(lua_State *L) {
	LuaClass<RText>::Register(L);
}
