#include "LuaBaseClass.h"
#include "LuaScript.h"
#include <map>

#ifdef _DEBUG_ARRAY
long long LuaBaseClass::idIterator = 1;
#endif

volatile long long LuaBaseClass::_instanceCount = 0;
#ifdef _DEBUG
#endif


char **__classNameMap = NULL;
int *__classCounts = NULL;
unsigned short int __classIterator = 1;

typedef char *pchar;
#define MAX_CLASS_COUNT 1024

typedef std::map<std::string, unsigned short int> NameMap;
NameMap nameMap;

unsigned short int LuaBaseClass::getClassId(const char *name) {

	if (__classNameMap == NULL) {
		__classNameMap = new pchar[MAX_CLASS_COUNT];
		__classCounts = new int[MAX_CLASS_COUNT];
	}
/*
	NameMap::iterator it = nameMap.find(name);
	if (it == nameMap.end()) {
		auto id = __classIterator;
		__classIterator++;

		// todo: check __classIterator with MAX_CLASS_COUNT

		nameMap.insert(std::pair<std::string, unsigned short int>(name, id));
		__classNameMap[id] = (char *)name;
		return id;
	}
	
	return it->second;
	*/

	auto it = nameMap.find(name);
	if (it == nameMap.end()) {
		auto id = __classIterator;
		__classIterator++;
		__classNameMap[id] = (char *)name;
		__classCounts[id] = 0;

		nameMap.insert(std::pair<std::string, unsigned short int>(name, id));
		return id;
	}

	return it->second;
}

int LuaBaseClass::getClassCountByName(const char *name) {
	auto it = nameMap.find(name);
	if (it == nameMap.end()) {
		return 0;
	}
	return __classCounts[it->second];
}

LuaBaseClass :: LuaBaseClass(unsigned short int classId) : _classId(classId), _lock(NULL), _share(1)
{
#ifdef _DEBUG_ARRAY
	_id = LuaBaseClass::idIterator++;
#endif
		// share(1);

	LuaBaseClass::_instanceCount++;
	__classCounts[_classId]++;

#ifdef _DEBUG
#endif
	// OutputDebugStringA("LuaBaseClass::LuaBaseClass\n");
}

// virtual ~LuaBaseClass() { if (_lock) delete _lock; }
LuaBaseClass::~LuaBaseClass() {
	if (_lock) {
		delete _lock;
	}
	LuaBaseClass::_instanceCount--;
	__classCounts[_classId]--;
#ifdef _DEBUG
#endif
}

int LuaBaseClass::_Wsetmt(lua_State *L) {
	auto bc = (LuaBaseClass *)((UserData *)lua_touserdata(L, 1))->data;
	bc->setmt();
	return 0;
}
