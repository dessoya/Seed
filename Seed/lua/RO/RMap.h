#pragma once

#include "..\..\LuaScript.h"
#include "..\..\LuaBaseClass.h"
#include "..\..\LuaClass.h"
#include "..\..\LuaCheck.h"
#include "RenderObject.h"
#include "..\DD\DrawMachine.h"

typedef unsigned short int CellID;
typedef struct {

	Image *image;
	int x, y;

} CellInfo;

typedef CellInfo *PCellInfo;

typedef struct {

	int w, h;
	double k;

} ScaleInfo;

LuaChildClass(RMap, RenderObject)

	RMap(unsigned short int classId);
	void __init(lua_State *L);
	~RMap();

	virtual void __drawSelf(DDDrawMachine *drawMachine);
	void _loadFromWorldMap(lua_State *L);

	LuaIMethod(setScaleInfo);
	LuaIMethod(setCellImage);
	LuaIMethod(setViewSize);
	LuaIMethod(setCoords);
	// LuaIMethod(setScale);

	friend DDDrawMachine;

private:

	PCellInfo *_images;
	ScaleInfo *_scaleInfo;
	int _curScale;
	CellID *_map;
	int _vw, _vh, _vwp, _vhp;
	int _cw, _ch;
	int _ox, _oy;
	int _scales, _cells;
	long long _mx, _my, _mxp, _myp;
};