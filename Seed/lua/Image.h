#pragma once

#include "..\LuaScript.h"
#include "..\LuaBaseClass.h"
#include "..\LuaClass.h"
#include "..\LuaCheck.h"

#pragma comment(lib, "zlibstat.lib")
#ifdef _DEBUG
#pragma comment(lib, "libpngd.lib")
#else
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "libzipmasm.lib ")
#endif

LuaClass(Image)

	Image(unsigned short int classId);
	void __init(lua_State *L);
	~Image();

	LuaIMethod(scale);
	void __scale(Image *source, DWORD m, DWORD d);

	
	unsigned int _width, _height;
	char *_data;

};
