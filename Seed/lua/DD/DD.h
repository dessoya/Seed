#pragma once

#include "windows.h"
#include "ddraw.h"

#pragma comment(lib, "ddraw.lib")

#include "..\..\LuaScript.h"
#include "..\..\LuaTypes.h"
#include "..\..\LuaBaseClass.h"
#include "..\..\LuaClass.h"
#include "..\..\LuaCheck.h"

const char *DDErrorString(HRESULT hr);

LuaClass(DD)

	DD(unsigned short int classId);
	void __init(lua_State *L);
	~DD() {	}

	LuaIMethod(setCooperativeLevel);
	LuaIMethod(getModeList);
	LuaIMethod(getScreenSize);
	LuaIMethod(setMode);
	
	LPDIRECTDRAW7 _getDD() { return _dd; }

private:

	LPDIRECTDRAW7 _dd;
};
