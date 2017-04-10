#pragma once

#include "..\..\LuaScript.h"
#include "..\..\LuaBaseClass.h"
#include "..\..\LuaClass.h"
#include "..\..\LuaCheck.h"
#include "..\Array.h"

#define ROF_ID				0
#define ROF_TABLEWRAP		1
#define ROF_CHILDS			2
#define ROF_DRAWORDER		3
#define ROF_X				4
#define ROF_Y				5
#define ROF_W				6
#define ROF_H				7
#define ROF_BX				8
#define ROF_BY				9
#define ROF_SKIPHOVER		10
#define ROF_ISHOVER			11
#define ROF_LPUSHSTATE		12

#define ROEVENTS			13
// events 

#define ROF_ONMOUSEMOVE		(ROEVENTS + 0)
#define ROF_ONHOVER			(ROEVENTS + 1)
#define ROF_ONHOVERLOST		(ROEVENTS + 2)
#define ROF_ONVIEWSIZE		(ROEVENTS + 3)
#define ROF_ONADD			(ROEVENTS + 4)
#define ROF_ONPARENTCHANGEPOS (ROEVENTS + 5)
#define ROF_LDOWN			(ROEVENTS + 6)
#define ROF_LUP				(ROEVENTS + 7)
#define ROF_LCLICK			(ROEVENTS + 8)

#define ROPROPS				(ROEVENTS + 9)

// rrect

#define RRF_COLOR			(ROPROPS + 0)

// rtext

#define RTF_TEXT			(ROPROPS + 0)
#define RTF_FONT			(ROPROPS + 1)
#define RTF_COLOR			(ROPROPS + 2) 

// rbox

#define RBF_COLOR			(ROPROPS + 0)
#define RBF_SIZE			(ROPROPS + 1)
#define RBF_RCOLOR			(ROPROPS + 2)

// rimage
#define RIF_IMAGE			(ROPROPS + 0)
#define RIF_IX				(ROPROPS + 1)
#define RIF_IY				(ROPROPS + 2)
#define RIF_IW				(ROPROPS + 3)
#define RIF_IH				(ROPROPS + 4)

// rmap
#define RMF_TERRAINMAP		(ROPROPS + 0)
#define RMF_SCALES			(ROPROPS + 1)
#define RMF_CELLS			(ROPROPS + 2)
#define RMF_CURSCALE		(ROPROPS + 3)
#define RMF_CURW			(ROPROPS + 4)
#define RMF_CURH			(ROPROPS + 5)



class DDDrawMachine;
LuaChildClass(RenderObject, Array)
public:
	RenderObject(unsigned short int classId);
	~RenderObject();

	void __init(lua_State *L);

	bool __isPointOn(long long x, long long y);

	// return object / table / nil
	LuaIMethod(isPointOn);
	RenderObject *__findHoveredObject(int x, int y);

	void _execEvent(lua_State *L, int eventIndex, int argsFrom, int argsTo);
	LuaIMethod(event);
	LuaIMethod(cevent);
	
	LuaIMethod(add);
	LuaIMethod(del);
	void __deleteChilds();

	// call self __draw and childs __draw ar table.draw
	LuaIMethod(draw);
	virtual void __draw(lua_State *L, DDDrawMachine *drawMachine);
	virtual void __drawSelf(DDDrawMachine *drawMachine);
};