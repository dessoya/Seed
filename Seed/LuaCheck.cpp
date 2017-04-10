#include "LuaCheck.h"
#include "LuaScript.h"
#include "LuaTypes.h"
#include "LuaBaseClass.h"

bool LC_checkUserDataClass(lua_State *L, int idx, const char *className) {

	auto ud = (UserData *)lua_touserdata(L, idx);
	// OutputDebugStringA("_checkUserDataClass 1\n");
	if (ud == NULL || ud->type != UDT_Class) {
		return false;
	}

	auto bc = (LuaBaseClass *)ud->data;

	/*
	char b[1024];
	sprintf(b, "_checkUserDataClass 2 %s\n", bc->_class);
	OutputDebugStringA(b);
	*/

	if (strcmp(bc->getClassName(), className) != 0) {
		return false;
	}
	// OutputDebugStringA("_checkUserDataClass 3\n");

	return true;
}