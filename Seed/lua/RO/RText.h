#pragma once

#include "..\..\LuaScript.h"
#include "..\..\LuaBaseClass.h"
#include "..\..\LuaClass.h"
#include "..\..\LuaCheck.h"
#include "RenderObject.h"

LuaChildClass(RText, RenderObject)

	RText(unsigned short int classId);
	void __init(lua_State *L);
	~RText();

	void __drawSelf(DDDrawMachine *drawMachine);

protected:

	// void checkArgument(lua_State *L, ArrayItem *item, int position);

};

