#include "Map.h"

Map::Map(unsigned short int classId) : LuaBaseClass(classId), queueList(0) { }

Map::~Map() {
	if (queueList) {
		delete queueList;
	}
}

LuaEMethod(Map, addQueue) {

	CHECK_ARGCLASS(1, Queue, _queue);

	if (_lock) _lock->lock();

	if (queueList == NULL) {
		queueList = new QueueList();
	}

	queueList->push_back(_queue);

	if (_lock) _lock->unlock();

	return 0;
}
	
LuaEMethod(Map, delQueue) {

	CHECK_ARGCLASS(1, Queue, _queue);

	if (_lock) _lock->lock();

	if (queueList) {
		auto it = queueList->begin(), eit = queueList->end();
		while (it != eit) {
			auto i = *it;
			if (_queue == i) {
				queueList->erase(it);
				break;
			}
			it++;
		}
		
	}

	if (_lock) _lock->unlock();

	return 0;
}


void Map::__pushValue(lua_State *L, ArrayItem &ai) {

	switch (ai.type) {
	case UDT_Integer: {
		lua_pushinteger(L, ai.i);
	}
	break;
	case UDT_Double: {
		lua_pushnumber(L, ai.d);
	}
	break;
	case UDT_Boolean: {
		lua_pushboolean(L, ai.b);
	}
	break;
	case UDT_Class: {
		auto bc = ai.baseClass;
		bc->share(1);
		auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
		ud->type = UDT_Class;
		ud->data = bc;
		bc->registerInNewState(L);
	}
	break;
	}
}

LuaEMethod(Map, foreach) {
	CHECK_ARG(1, function);

	if (_lock) _lock->lock_shared();

	auto it = arrayItemMap.begin(), eit = arrayItemMap.end();
	while (it != eit) {
		auto ai = it->second;

		lua_pushvalue(L, 1);
		lua_pushlstring(L, it->first.c_str(), it->first.length());
		__pushValue(L, ai);
		lua_call(L, 2, 0);
		it++;
	}

	if (_lock) _lock->unlock_shared();

	return 0;
}


LuaEMethod(Map, get) {
	CHECK_ARG(1, string);
	if(_lock) _lock->lock_shared();
	auto it = arrayItemMap.find(lua_tostring(L, 1));
	if (it == arrayItemMap.end()) {
		lua_pushnil(L);
	}
	else {
		__pushValue(L, it->second);
	}
	if (_lock) _lock->unlock_shared();
	return 1;
}

LuaEMethod(Map, set) {
	CHECK_ARG(1, string);
	auto name = lua_tostring(L, 1);
	ArrayItem ai;

	auto tp = lua_type(L, 2);
	// LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA.
	switch (tp) {
	case LUA_TNUMBER: {
		auto ni = lua_tointeger(L, 2);
		auto nd = lua_tonumber(L, 2);
		if (ni == nd) {
			ai.type = UDT_Integer;
			ai.i = ni;
		}
		else {
			ai.type = UDT_Double;
			ai.d = nd;
		}
	}
	break;
	case LUA_TBOOLEAN: {
		ai.type = UDT_Boolean;
		ai.b = lua_toboolean(L, 2);
	}
	break;
	case LUA_TUSERDATA: {
		auto ud = (UserData *)lua_touserdata(L, 2);
		auto bc = (LuaBaseClass *)ud->data;
		ai.type = UDT_Class;
		bc->share(1);
		ai.baseClass = bc;
	}
	break;
	default:
		lua_pushstring(L, "Map:set value type error");
		lua_error(L);
	}


	if (_lock) _lock->lock();
	// todo: check exists
	auto it = arrayItemMap.find(name);
	if (it == arrayItemMap.end()) {
		arrayItemMap.insert(std::pair<std::string, ArrayItem>(name, ai));
	}
	else {
		it->second = ai;
	}

	if (queueList == NULL) {
		queueList = new QueueList();
	}

	if (queueList) {
		for (auto it = queueList->begin(), eit = queueList->end(); it != eit; it++) {
			auto q = *it;
			lua_pushcfunction(L, Queue::_Wsend);

			auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
			ud->type = UDT_Class;
			ud->data = q;
			q->share(1);
			q->registerInNewState(L);

			Array *a = LuaClass<Array>::__create(L);
			// Array *a = new Array(Array::__classId);
			ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
			ud->type = UDT_Class;
			ud->data = a;
			dynamic_cast<LuaBaseClass *>(a)->registerInNewState(L);

			lua_call(L, 2, 0);
		}
	}

	if (_lock) _lock->unlock();

	return 0;
}

LuaMethods(Map) = {
	LuaMethodDesc(Map, get),
	LuaMethodDesc(Map, set),
	LuaMethodDesc(Map, addQueue),
	LuaMethodDesc(Map, delQueue),
	LuaMethodDesc(Map, foreach),
	{ 0,0 }
};

LuaMetaMethods(Map) = {
	{ 0,0 }
};

void module_map(lua_State *L) {
	LuaClass<Map>::Register(L);
}