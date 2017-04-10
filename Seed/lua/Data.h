#pragma once

#include "..\LuaScript.h"
#include "..\LuaBaseClass.h"
#include "..\LuaClass.h"
#include "..\LuaCheck.h"

typedef void *(*LoadDataCallback)(const char *filename, size_t *sz);
extern LoadDataCallback loadDataCallback;

LuaClass(Data)

	Data(unsigned short int classId);
	void __init(lua_State *L);
	~Data();

	void __set(void *d, size_t sz);
	LuaIMethod(tostring);	
	LuaIMethod(saveToFile);

	// LuaIMethod(set);
	void *_data;
	size_t _size;

};
