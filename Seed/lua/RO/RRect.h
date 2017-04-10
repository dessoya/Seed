#pragma once

#include "..\..\LuaScript.h"
#include "..\..\LuaBaseClass.h"
#include "..\..\LuaClass.h"
#include "..\..\LuaCheck.h"
#include "RenderObject.h"

LuaChildClass(RRect, RenderObject)

	RRect(unsigned short int classId);
	void __init(lua_State *L);
	~RRect();

	void __drawSelf(DDDrawMachine *drawMachine);

protected:

	void checkArgument(lua_State *L, ArrayItem *item, int position);

};

