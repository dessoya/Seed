#include "TerrainMap.h"

TerrainMapBlock::TerrainMapBlock(unsigned short int classId) : LuaBaseClass(classId) {
	_childs = 0;	
	_block = new CellID[1 << (B_BITS + B_BITS)];
	memset(_block, 0, sizeof(CellID) * (1 << (B_BITS + B_BITS)));
}

TerrainMapBlock::~TerrainMapBlock() {
	delete _block;
}

void TerrainMapBlock::__init(lua_State *L) {
	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	_x = lua_tointeger(L, 1);
	_y = lua_tointeger(L, 2);
}


CellID TerrainMapBlock::__get(lint x, lint y) {
	lint x1 = x - _x;
	lint y1 = y - _y;
	return _block[x1 + (y1 << B_BITS)];
}

LuaEMethod(TerrainMapBlock, set) {
	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	CHECK_ARG(3, integer);	
	lint x1 = lua_tointeger(L, 1) - _x;
	lint y1 = lua_tointeger(L, 2) - _y;
	_block[x1 + (y1 << B_BITS)] = (CellID)lua_tointeger(L, 3);
	return 0;
}



LuaMethods(TerrainMapBlock) = {
	LuaMethodDesc(TerrainMapBlock, set),
	{ 0,0 }
};

LuaMetaMethods(TerrainMapBlock) = {
	{ 0,0 }
};



// ------------------------------------------------------------------------------------------------




TerrainMapScaleBlock::TerrainMapScaleBlock(unsigned short int classId) : Array(classId) {
}

TerrainMapScaleBlock::~TerrainMapScaleBlock() {
}

TerrainMapScaleBlock *TerrainMapScaleBlock::__getBlock(lint x, lint y) {

	lint x1 = (lint(x - _x)) >> (lint)_m;
	lint y1 = (lint(y - _y)) >> (lint)_m;
	auto p = getData();
	p = &p[x1 + (y1 << B_BITS)];
	if (p->type == UDT_Nil) {
		return 0;
	}
	return (TerrainMapScaleBlock *)p->baseClass;
}

TerrainMapScaleBlock *TerrainMapScaleBlock::__addBlock(lua_State *L, lint x, lint y) {

	auto x1 = (x - _x) >> _m;
	auto y1 = (y - _y) >> _m;

	auto _x1 = (x1 << _m) + _x;
	auto _y1 = (y1 << _m) + _y;

	TerrainMapScaleBlock *b;

	if (_level == 1) {
		auto _b = LuaClass<TerrainMapBlock>::__create(L);
		b = (TerrainMapScaleBlock *)_b;

		lua_pushcfunction(L, [](lua_State *L) -> int {
			auto ud = (UserData *)lua_touserdata(L, 1);
			lua_remove(L, 1);
			auto b = (TerrainMapBlock *)ud->data;
			b->__init(L);
			return 0;
		});

		auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
		ud->type = UDT_Class;
		ud->data = _b;
		_b->share(1);
		_b->share(1);

		lua_pushinteger(L, _x1);
		lua_pushinteger(L, _y1);
		lua_call(L, 3, 0);
	}
	else {
		b = LuaClass<TerrainMapScaleBlock>::__create(L);

		lua_pushcfunction(L, [](lua_State *L) -> int {
			auto ud = (UserData *)lua_touserdata(L, 1);
			lua_remove(L, 1);
			auto b = (TerrainMapScaleBlock *)ud->data;
			b->__init(L);
			return 0;
		});

		auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
		ud->type = UDT_Class;
		ud->data = b;
		b->share(1);
		b->share(1);

		lua_pushinteger(L, _level - 1);
		lua_pushinteger(L, _x1);
		lua_pushinteger(L, _y1);
		lua_call(L, 4, 0);
	}

	int pos = (int)(x1 + (y1 << B_BITS));

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Class;
	ud->data = b;
	auto i = lua_gettop(L);

	_helper_set(L, i, i, pos);
	lua_pop(L, 1);

	return b;
}


bool TerrainMapScaleBlock::__isArea(lint x, lint y) {
	if (x < _x || y < _y || x > _xe || y > _ye) {
		return false;
	}
	return true;
}


void TerrainMapScaleBlock::__init(lua_State *L) {
	
	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	CHECK_ARG(3, integer);

	_level = (unsigned short int)lua_tointeger(L, 1);
	_x = lua_tointeger(L, 2);
	_y = lua_tointeger(L, 3);

	lint size = ((lint)1) << (_level * B_BITS);
	_xe = _x + size - 1;
	_ye = _y + size - 1;

	lua_pushnil(L);
	auto nilindex = lua_gettop(L);	
	_helper_set(L, nilindex, nilindex, (1 << (B_BITS + B_BITS)) - 1);
	lua_pop(L, 1);
}



LuaAMethods(TerrainMapScaleBlock, [](lua_State *L, int methods) { LuaAddMethods(Array); }) = {
	{ 0,0 }
};

LuaMetaMethods(TerrainMapScaleBlock) = {
	{ 0,0 }
};



// ------------------------------------------------------------------------------------------------




TerrainMap::TerrainMap(unsigned short int classId) : LuaBaseClass(classId) {
	_level = 1;
	setmt();
}

void TerrainMap::__init(lua_State *L) {
	_root = LuaClass<TerrainMapScaleBlock>::__create(L);

	lua_pushcfunction(L, [](lua_State *L) -> int {
		auto ud = (UserData *)lua_touserdata(L, 1);
		lua_remove(L, 1);
		auto b = (TerrainMapScaleBlock *)ud->data;
		b->__init(L);
		return 0;
	});

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Class;
	ud->data = _root;
	_root->share(1);

	lua_pushinteger(L, _level);
	lua_pushinteger(L, MAP_MID);
	lua_pushinteger(L, MAP_MID);
	lua_call(L, 4, 0);
}


TerrainMap::~TerrainMap() {
}

LuaEMethod(TerrainMap, expand) {
	
	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);

	lint x = lua_tointeger(L, 1);
	lint y = lua_tointeger(L, 2);

	_lock->lock();

	while (true) {
		if(!_root->__isArea(x, y)) {

			_level++;

			auto e = ( ((lint)1) << ((B_BITS * _level) - 1 + B_BITS) );

			auto o = _root;

			_root = LuaClass<TerrainMapScaleBlock>::__create(L);

			lua_pushcfunction(L, [](lua_State *L) -> int {
				auto ud = (UserData *)lua_touserdata(L, 1);
				lua_remove(L, 1);
				auto b = (TerrainMapScaleBlock *)ud->data;
				b->__init(L);
				return 0;
			});

			auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
			ud->type = UDT_Class;
			ud->data = _root;
			_root->share(1);

			lua_pushinteger(L, _level);
			lua_pushinteger(L, MAP_MID - e);
			lua_pushinteger(L, MAP_MID - e);
			lua_call(L, 4, 0);

			ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
			ud->type = UDT_Class;
			ud->data = o;
			o->share(1);
			auto i = lua_gettop(L);
			auto x1 = ((lint)1) << (B_BITS - 1);
			_root->_helper_set(L, i, i, (int)(x1 + (x1 << B_BITS)));
			lua_pop(L, 1);

			continue;
		}
		break;
	}

	_lock->unlock();

	return 0;
}

/*
LuaEMethod(TerrainMap, getRoot) {

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->data = _root;
	ud->type = UDT_Class;
	_root->share(1);
	
	return 1;
}
*/

LuaEMethod(TerrainMap, getBlock) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);

	lint x = lua_tointeger(L, 1);
	lint y = lua_tointeger(L, 2);

	lint level = _level;
	auto b = _root;

	while (level > 0 && b) {

		auto bb = b;
		b = b->__getBlock(x, y);

		if (b == NULL) {
			b = bb->__addBlock(L, x, y);
		}

		level--;
	}

	TerrainMapBlock *l = (TerrainMapBlock *)b;

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Class;
	ud->data = l;
	l->share(1);
	l->registerInNewState(L);

	return 1;
}
	

CellID TerrainMap::__getCell(lint x, lint y) {

	if (!_root->__isArea(x, y)) {
		return ABSENT_CELL;
	}

	auto level = _level;
	auto b = _root;

	while (level > 0 && b) {

		b = b->__getBlock(x, y);

		level--;
	}

	if (b == NULL) {
		return ABSENT_CELL;
	}

	TerrainMapBlock *l = (TerrainMapBlock *)b;

	return l->__get(x, y);
}

LuaEMethod(TerrainMap, getCell) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);

	lint x = lua_tointeger(L, 1);
	lint y = lua_tointeger(L, 2);

	auto cell = __getCell(x, y);

	lua_pushinteger(L, cell & 0x0fff);
	lua_pushinteger(L, (cell & 0xf000) >> 12);

	return 2;
}


LuaMethods(TerrainMap) = {
	LuaMethodDesc(TerrainMap, expand),
	LuaMethodDesc(TerrainMap, getCell),
	LuaMethodDesc(TerrainMap, getBlock),	
	{ 0,0 }
};

LuaMetaMethods(TerrainMap) = {
	{ 0,0 }
};

void module_terrain_map(lua_State *L) {	
	LuaClass<TerrainMapBlock>::Register(L);
	LuaClass<TerrainMapScaleBlock>::Register(L);
	LuaClass<TerrainMap>::Register(L);
}