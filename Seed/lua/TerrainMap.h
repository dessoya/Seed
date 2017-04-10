#pragma once

#include "..\LuaScript.h"
#include "..\LuaBaseClass.h"
#include "..\LuaClass.h"
#include "..\LuaCheck.h"
#include "Array.h"

typedef unsigned long long lint;
typedef unsigned short int CellID;

#define B_BITS			5
#define ABSENT_CELL		0x0fff
#define MAP_MID			0x800000000

LuaClass(TerrainMapBlock)
	TerrainMapBlock(unsigned short int classId);
	~TerrainMapBlock();
	void __init(lua_State *L);
	CellID __get(lint x, lint y);
	LuaIMethod(set);
private:
	Array *_childs;
	lint _x, _y;
	CellID *_block;
};

LuaChildClass(TerrainMapScaleBlock, Array)
	TerrainMapScaleBlock(unsigned short int classId);
	~TerrainMapScaleBlock();
	void __init(lua_State *L);
	TerrainMapScaleBlock *__getBlock(lint x, lint y);
	TerrainMapScaleBlock *__addBlock(lua_State *L, lint x, lint y);
	bool __isArea(lint x, lint y);
private:
	unsigned short int _level;
	unsigned short int _m;
	lint _x, _y, _xe, _ye;

};

LuaClass(TerrainMap)

	TerrainMap(unsigned short int classId);
	~TerrainMap();
	void __init(lua_State *L);

	LuaIMethod(expand);
	LuaIMethod(getCell);
	LuaIMethod(getBlock);

	CellID __getCell(lint x, lint y);

private:
	unsigned short int _level;
	TerrainMapScaleBlock *_root;
};