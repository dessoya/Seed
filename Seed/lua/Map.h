#pragma once

#include "..\LuaScript.h"
#include "..\LuaBaseClass.h"
#include "..\LuaClass.h"
#include "..\LuaCheck.h"
#include "Array.h"
#include "Queue.h"
#include <string>
#include <map>
#include <list>

LuaClass(Map)

	Map(unsigned short int classId);
	~Map();

	LuaIMethod(get);
	LuaIMethod(set);
	LuaIMethod(addQueue);	
	LuaIMethod(delQueue);	
	LuaIMethod(foreach);	
	void __pushValue(lua_State *L, ArrayItem &ai);

	typedef std::map<std::string, ArrayItem> ArrayItemMap;
	ArrayItemMap arrayItemMap;

	typedef std::list<Queue *> QueueList;
	QueueList *queueList;

};
