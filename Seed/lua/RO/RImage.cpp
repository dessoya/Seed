#include "RImage.h"
#include "..\DD\DrawMachine.h"
#include "..\Windows.h"

RImage::RImage(unsigned short int classId) : RenderObject(classId) {
}

void RImage::__init(lua_State *L) {

	RenderObject::__init(L);

	auto c = lua_gettop(L);
	// check params	
	if (c < 7) {
		luaL_where(L, 1);
		lua_pushfstring(L, "%RText need 7 arguments but have %d", lua_tostring(L, -1), getCount());
		lua_remove(L, -2);
		lua_error(L);
	}

	_helper_set(L, 1, 2, ROF_X);
	_helper_set(L, 3, 7, RIF_IMAGE);

	auto p = getData();

	p[ROF_BX].type = UDT_Integer;
	p[ROF_BX].i = p[ROF_X].i;

	p[ROF_BY].type = UDT_Integer;
	p[ROF_BY].i = p[ROF_Y].i;

	p[ROF_W].type = UDT_Integer;
	p[ROF_W].i = p[RIF_IW].i;

	p[ROF_H].type = UDT_Integer;
	p[ROF_H].i = p[RIF_IH].i;

	/*
	auto p = getData();
	for (int i = 0; i < 5; i++, p++) {
	checkArgument(L, p, i);
	}
	*/
}

RImage::~RImage() {

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


void RImage::__drawSelf(DDDrawMachine *drawMachine) {

	auto p = getData();

	auto _x = p[ROF_X].i;
	auto _y = p[ROF_Y].i;
	auto _fromx = p[RIF_IX].i;
	auto _fromy = p[RIF_IY].i;
	auto _w = p[RIF_IW].i;
	auto _h = p[RIF_IH].i;
	auto _image = (Image *)p[RIF_IMAGE].baseClass;

	drawMachine->__drawImage((int)_x, (int)_y, _image, (int)_fromx, (int)_fromy, (int)_w, (int)_h);
}

LuaAMethods(RImage, [](lua_State *L, int methods) { LuaAddMethods(RenderObject); }) = {
	// LuaMethodDesc(RRect, get),
	{ 0,0 }
};

LuaMetaMethods(RImage) = {
	// LuaMethodDesc(Array, __tostring),
	// LuaMethodDesc(Font, __newindex),
	{ 0,0 }
};


void module_rimage(lua_State *L) {
	LuaClass<RImage>::Register(L);
}