#pragma once

#include "..\LuaScript.h"
#include "..\LuaBaseClass.h"
#include "..\LuaClass.h"
#include "..\LuaCheck.h"

class ArrayItem {
public:
	unsigned long long type : 8, len : 24;
	union {
		bool b;
		double d;
		long long i;
		const char *str;
		LuaBaseClass *baseClass;
	};
};

LuaClass(Array)

	Array(unsigned short int classId);
	void __init(lua_State *L);
	~Array();

	int _helper_set(lua_State *L, int index, int count, int setPosition);
		
	LuaIMethod(get);
	LuaIMethod(set);
	LuaIMethod(push);
	LuaIMethod(count);

	ArrayItem *getData() {
		return _data;
	}

	int getCount() {
		return _count;
	}

private:
	unsigned short int _size, _count;
	ArrayItem *_data;

protected:

	// void __setSize(unsigned short int newSize);

	virtual void checkArgument(lua_State *L,ArrayItem *item, int position);


};

