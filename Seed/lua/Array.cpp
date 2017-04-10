#include "Array.h"
#include "windows.h"

Array::Array(unsigned short int classId) : LuaBaseClass(classId) {

	_count = _size = 0;
	_data = NULL;

}

void Array::__init(lua_State *L) {

	auto c = lua_gettop(L);
	if (c == 0) {
		_size = 8;
		_data = new ArrayItem[_size];
		return;
	}

	_helper_set(L, 1, c, 0);
}

Array::~Array() {
	 // OutputDebugStringA("~Array\n");


	auto p = _data;
	for (int i = 0; i < _count; i++, p++) {
		switch (p->type) {
		case UDT_Function:
		case UDT_Table:
			luaL_unref(_currentState, LUA_REGISTRYINDEX, (int)p->i);
			break;
		case UDT_String:
			delete p->str;
			break;
		case UDT_Class:
			auto share = p->baseClass->share(-1);
			if (share == 0) {
				// _currentState = L;
				delete p->baseClass;
			}
			break;
		}
	}
	delete _data;

}

LuaEMethod(Array, count) {
	lua_pushinteger(L, _count);
	return 1;
}

LuaEMethod(Array, set) {
	CHECK_ARG(1, integer);
	/*
	char b[1024];
	sprintf_s(b, "2 type %s\n", lua_typename(L, lua_type(L, 2)) );
	OutputDebugStringA(b);
	*/
	return _helper_set(L, 2, lua_gettop(L), (int)lua_tointeger(L, 1));
}

LuaEMethod(Array, push) {
	return _helper_set(L, 1, lua_gettop(L), _count);
}

void Array::checkArgument(lua_State *L,ArrayItem *item, int position) { }

/*
void Array::__setSize(unsigned short int newSize) {

}
*/

int Array::_helper_set(lua_State *L, int index, int count, int setPosition) {

	/*
	char b[1024];
	sprintf_s(b, "_helper_set top %d, index %d, count %d, setPosition %d\n", lua_gettop(L), index, count, setPosition);
	OutputDebugStringA(b);
	*/
	// CHECK_ARG(1, integer);

	if (_lock) _lock->lock();

	auto newSize = setPosition + ( count - index + 1);
	auto setCount = newSize;

	if (newSize > _size) {
		// realoc
		newSize = (newSize / 8 + (newSize % 8 ? 1 : 0)) * 8;
		ArrayItem *i = new ArrayItem[newSize];
		if (_count) {
			memcpy(i, _data, sizeof(ArrayItem) * _count);
		}
		if (_data) {
			delete _data;
		}
		_data = i;
		_size = newSize;
	}

	if (_count < setPosition) {
		auto d = &_data[_count];
		for (int i = _count; i < setPosition; i++, d++) {
			d->type = UDT_Nil;
		}
	}

	// auto l = setPosition + count;
	if (setCount > _count) {
		_count = setCount;
	}

	//sprintf_s(b, "_size %d, _count %d\n", _size, _count);
	//OutputDebugStringA(b);


	// let's load
	size_t sz;
	const char *s;
	UserData *ud;
	long long ni;
	double nd;

	auto d = &_data[setPosition];
	for (int i = index, ie = count; i <= ie; i++, d++, setPosition++) {
		auto tp = lua_type(L, i);

		//sprintf_s(b, "i %d type %s\n", i, lua_typename(L, tp));
		//OutputDebugStringA(b);

		switch (tp) {
		// LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA.
		case LUA_TNIL:
			d->type = UDT_Nil;
			break;
		case LUA_TNUMBER:
			ni = lua_tointeger(L, i);
			nd = lua_tonumber(L, i);
			if (ni == nd) {
				d->type = UDT_Integer;
				d->i = ni;
			}
			else {
				d->type = UDT_Double;
				d->d = nd;
			}
			break;
		case LUA_TBOOLEAN:
			d->type = UDT_Boolean;
			d->b = lua_toboolean(L, i);
			break;
		case LUA_TSTRING:
			d->type = UDT_String;
			s = lua_tolstring(L, i, &sz);
			d->str = new char[sz + 1];
			memcpy((void *)d->str, s, sz + 1);
			d->len = sz;
			break;
		case LUA_TFUNCTION:
			d->type = UDT_Function;
			// lua_insert(L, i);
			lua_pushvalue(L, i);
			d->i = luaL_ref(L, LUA_REGISTRYINDEX);
			break;
		case LUA_TTABLE:
			d->type = UDT_Table;
			// lua_insert(L, i);
			lua_pushvalue(L, i);
			d->i = luaL_ref(L, LUA_REGISTRYINDEX);
			break;
		case LUA_TUSERDATA:
			ud = (UserData *)lua_touserdata(L, i);
			d->type = ud->type;
			switch(d->type) {
			case UDT_Class:
				d->baseClass = (LuaBaseClass *)ud->data;
				d->baseClass->share(1);
				break;
			default:
				if (_lock) _lock->unlock();
				lua_pushfstring(L, "Array unknown UserData type %d", d->type);
				lua_error(L);
			}
			break;
		default:
			if (_lock) _lock->unlock();
			lua_pushfstring(L, "Array unsupport type %s", lua_typename(L, tp));
			lua_error(L);
		}
		checkArgument(L, d, setPosition);
	}
	if (_lock) _lock->unlock();

	return 0;
}

LuaEMethod(Array, get) {

	// OutputDebugStringA("array:get\n");

	if (_lock) _lock->lock_shared();

	auto t = lua_gettop(L);
	int c, p;
	bool _forceLoadModules = false;

	if (t == 0) {
		p = 0;
		c = _count;
	}
	else {
		p = (int)lua_tointeger(L, 1);
		if (t > 1) {
			c = (int)lua_tointeger(L, 2);
			if (c == -1) {
				c = _count - p;
				/*
				char b[1024];
				sprintf_s(b, "array:get -1 size avail %d, p %d, _count %d\n", c, p, _count);
				OutputDebugStringA(b);
				*/
			}
			if (t > 2) {
				_forceLoadModules = lua_toboolean(L, 3);
			}
		}
		else {
			c = 1;
		}
	}

	/*
	if (_forceLoadModules) {
		OutputDebugStringA("_forceLoadModules\n");
	}
	*/

	UserData *ud;
	LuaScript *script = NULL;

	// todo: check for bound
	auto d = &_data[p];
	for (int i = p, l = p + c; i < l; i++, d++) {

		if (p >= _count) {
			lua_pushnil(L);
			continue;
		}

		switch (d->type) {
		case UDT_Nil:
			lua_pushnil(L);
			break;
		case UDT_Integer:
			lua_pushinteger(L, d->i);
			break;
		case UDT_Double:
			lua_pushnumber(L, d->d);
			break;
		case UDT_String:
			lua_pushlstring(L, d->str, d->len);
			break;
		case UDT_Boolean:
			lua_pushboolean(L, d->b);
			break;
		case UDT_Function:
		case UDT_Table:
			lua_rawgeti(L, LUA_REGISTRYINDEX, (int)d->i);
			break;
		case UDT_Class:
			if (_forceLoadModules) {

				if (script == NULL) {
					lua_getglobal(L, "__CClass");
					script = (LuaScript *)lua_touserdata(L, -1);
					lua_pop(L, 1);
				}

				script->installModuleForClass(d->baseClass->_classId);
			}

			ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
			ud->type = UDT_Class;
			ud->data = d->baseClass;
			d->baseClass->share(1);

			d->baseClass->registerInNewState(L);
		}
	}

	if(_lock) _lock->unlock_shared();

	// OutputDebugStringA("array:get done\n");

	return c;
}

/*
int z = sizeof(LuaBaseClass);
int a = sizeof(Array);
*/

LuaMethods(Array) = {
	LuaMethodDesc(Array, push),
	LuaMethodDesc(Array, get),
	LuaMethodDesc(Array, set),
	LuaMethodDesc(Array, count),
	{ 0,0 }
};

LuaMetaMethods(Array) = {
	// LuaMethodDesc(Queue, __tostring),
	// LuaMethodDesc(Queue, __newindex),
	{ 0,0 }
};

LuaBaseClass *commonArray = NULL;

/*
i 0 = 3
i 1 = 10
i 2 = 15
i 3 = 20
i 4 = NULL
i 5 = NULL
i 6 = NULL
*/

/*
#include <algorithm>
#include <array>

#include <boost/sort/spreadsort/spreadsort.hpp>
using namespace boost::sort::spreadsort;

void a() {

	int a1 = 10, a2 = 20, a3 = 3, a4 = 15;
	int *s2[] = { &a1, 0, &a3, 0, &a2, &a4, 0, &a1, 0, &a3, 0, &a2, &a4, 0, &a1, 0, &a3, 0, &a2, &a4, 0, &a1, 0, &a3, 0, &a2, &a4, 0 };
	int *s3[] = { &a1, 0, &a3, 0, &a2, &a4, 0, &a1, 0, &a3, 0, &a2, &a4, 0, &a1, 0, &a3, 0, &a2, &a4, 0, &a1, 0, &a3, 0, &a2, &a4, 0 };

	static int ccount, scount, rshift;

	auto cfunc = [](int *a, int *b) {

		// if (a == NULL && b == NULL) return false;
		// if (a == NULL && b != NULL) return false;
		// if (a != NULL && b == NULL) return true;

		ccount++;
		if (a == NULL) return false;
		if (b == NULL) {
			scount++;
			return true;
		}

		if (*b > *a) {
			scount++;
			return true;
		}

		return false;
	};

	char b[1024];


	struct rightshift {
		inline int operator()(int *x, unsigned offset) { rshift++;  if (x) return *x >> offset; return 0; }
	};

	struct lessthan {
		bool operator()(int *x, int *y) const {
			ccount++;
			if (x == NULL) return false;
			if (y == NULL) {
				scount++;
				return true;
			}

			if (*x > *y) {
				scount++;
				return true;
			}

			return false;
		}
	};
	
	ccount = scount = rshift = 0;
	integer_sort(&s3[0], &s3[28], rightshift(), lessthan());

	sprintf_s(b, "ccount %d, scount %d, rshift %d\n", ccount, scount, rshift);
	OutputDebugStringA(b);

	for (int i = 0; i < 28; i++) {
		if (s3[i] == NULL) {
			sprintf_s(b, "i %d = NULL\n", i);
		}
		else {
			sprintf_s(b, "i %d = %d\n", i, *s3[i]);
		}
		OutputDebugStringA(b);
	}


	// ccount 22, scount 6
	// ccount 12, scount 0

	ccount = scount = 0;
	std::sort(&s2[0], &s2[28], cfunc);
	sprintf_s(b, "ccount %d, scount %d\n", ccount, scount);
	OutputDebugStringA(b);

	ccount = scount = 0;
	std::sort(&s2[0], &s2[28], cfunc);
	sprintf_s(b, "ccount %d, scount %d\n", ccount, scount);
	OutputDebugStringA(b);

	for (int i = 0; i < 28; i++) {
		if (s2[i] == NULL) {
			sprintf_s(b, "i %d = NULL\n", i);
		}
		else {
			sprintf_s(b, "i %d = %d\n", i, *s2[i]);
		}
		OutputDebugStringA(b);
	}
	
}
*/

void module_array(lua_State *L) {

	// a();

	LuaClass<Array>::Register(L);

	if (commonArray == NULL) {

		// OutputDebugStringA("create commonArray\n");
		auto sv = lua_gettop(L);
		lua_settop(L, 0);
		commonArray = LuaClass<Array>::__create(L);
		commonArray->share(1);
		commonArray->setmt();
		lua_settop(L, sv);

	}
}
