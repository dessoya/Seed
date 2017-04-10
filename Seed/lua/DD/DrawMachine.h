#pragma once

#include "..\..\LuaScript.h"
#include "..\..\LuaTypes.h"
#include "..\..\LuaBaseClass.h"
#include "..\..\LuaClass.h"
#include "..\..\LuaCheck.h"
#include "..\Windows.h"
#include "ddraw.h"
#include "Output.h"
#include "..\Image.h"

class RMap;

LuaClass(DDDrawMachine)

	friend RMap;

	DDDrawMachine(unsigned short int classId);
	~DDDrawMachine() { }

	void __lock();
	void __unlock();

	void __drawText(int x, int y, const char *text, HFONT font, DWORD color);
	void __drawRect(int x, int y, int w, int h, DWORD color);
	void __drawRect_pure(int x, int y, int w, int h, DWORD color);
	void __drawImage(int x, int y, Image *image, int fromx, int fromy, int w, int h);

	LuaIMethod(drawText);
	LuaIMethod(setOutput);
	LuaIMethod(drawRect);
	LuaIMethod(width);
	LuaIMethod(height);
	
private:

	int _pitch;
	unsigned char *_surface;
	DDOutput *_output;
};
