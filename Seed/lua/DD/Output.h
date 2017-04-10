#pragma once

#include "..\..\LuaScript.h"
#include "..\..\LuaTypes.h"
#include "..\..\LuaBaseClass.h"
#include "..\..\LuaClass.h"
#include "..\..\LuaCheck.h"
#include "..\Windows.h"
#include "ddraw.h"

#define OT_Windowed			1
#define OT_Fullscreen		2

LuaClass(DDOutput)

	DDOutput(unsigned short int classId);
	void __init(lua_State *L);

	~DDOutput() { }

	LuaIMethod(create);
	LuaIMethod(release);
	LuaIMethod(flip);
	LuaIMethod(isValid);
	LuaIMethod(getSize);
	
	void __checkSurfaces();

	// LuaIMethod(title);
	LPDIRECTDRAWSURFACE7 _dds;
	int _width, _height;

private:
	int _type;
	Window *_window;

	// common
	LPDIRECTDRAWSURFACE7 _dds_Primary;

	// windowed
	int _backSurfaceCount, _backSurfaceIndex;
	LPDIRECTDRAWSURFACE7 *_dds_Back;
	LPDIRECTDRAWCLIPPER _ddc_Clipper;

	// fullscreen
	LPDIRECTDRAWSURFACE7 _dds_backFullScreen;

};
