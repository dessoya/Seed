#pragma once

#include "..\..\LuaScript.h"
#include "..\..\LuaBaseClass.h"
#include "..\..\LuaClass.h"
#include "..\..\LuaCheck.h"
#include "RenderObject.h"

LuaChildClass(RImage, RenderObject)

	RImage(unsigned short int classId);
	void __init(lua_State *L);
	~RImage();

	virtual void __drawSelf(DDDrawMachine *drawMachine);

protected:

};

