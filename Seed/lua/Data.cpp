#include "Data.h"

Data::Data(unsigned short int classId) : LuaBaseClass(classId), _data(0), _size(0) {
	setmt();
}

void Data::__init(lua_State *L) {
	if (lua_gettop(L) == 0) return;

	auto s = lua_tolstring(L, 1, &_size);
	_data = new char[_size];
	memcpy(_data, s, _size);
}

Data::~Data() {
	// OutputDebugStringA("~Data\n");
	if (_data) {
		delete _data;
	}
}

void Data::__set(void *d, size_t sz) {
	_lock->lock();
	_data = d;
	_size = sz;
	_lock->unlock();
}

LuaEMethod(Data, tostring) {

	_lock->lock_shared();
	if (_data == NULL) {
		lua_pushnil(L);
	}
	else {
		lua_pushlstring(L, (const char *)_data, _size);
	}
	_lock->unlock_shared();

	return 1;
}

LuaEMethod(Data, saveToFile) {
	CHECK_ARG(1, string);
	FILE *f;
	_lock->lock_shared();
	fopen_s(&f, lua_tostring(L, 1), "wb");
	fwrite(_data, _size, 1, f);
	fclose(f);
	_lock->unlock_shared();

	return 0;
}
/*
LuaEMethod(Cursor, set) {
	if (_cursor) {
		SetCursor(_cursor);
	}
	return 0;
}
*/

LuaMethods(Data) = {
	LuaMethodDesc(Data, tostring),
	LuaMethodDesc(Data, saveToFile),
	{ 0,0 }
};

LuaMetaMethods(Data) = {
	// LuaMethodDesc(Array, __tostring),
	// LuaMethodDesc(Cursor, __newindex),
	{ 0,0 }
};

LoadDataCallback loadDataCallback = NULL;

static int luaC_loadFileData(lua_State *L) {
	
	CHECK_ARG(1, string);

	auto fn = lua_tostring(L, 1);

	void *_data = NULL;
	size_t sz;
	if (loadDataCallback) {
		_data = loadDataCallback(fn, &sz);
	}
	// check in callback

	if (_data == NULL) {
		FILE *f;
		fopen_s(&f, fn, "rb");
		if (f == NULL) {
			lua_pushfstring(L, "can't open file '%s'", fn);
			lua_error(L);
		}

		fseek(f, 0, SEEK_END);
		sz = ftell(f);
		fseek(f, 0, SEEK_SET);
		_data = (void *)(new char[sz]);
		fread(_data, sz, 1, f);

		fclose(f);
	}

	auto data = LuaClass<Data>::__create(L);
	data->__set(_data, sz);

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Class;
	ud->data = data;
	data->registerInNewState(L);
	
	return 1;
}


void module_data(lua_State *L) {
	LuaClass<Data>::Register(L);
	lua_register(L, "loadFileData", luaC_loadFileData);
}