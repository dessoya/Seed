#include "RenderObject.h"
#include "windows.h"

long long renderObjectIterator = 1;

RenderObject::RenderObject(unsigned short int classId) : Array(classId) {
	// _lid = -1;
 	setmt();
}

RenderObject::~RenderObject() {

	/*
	OutputDebugStringA("~RenderObject\n");

	auto p = getData();
	if (p[ROF_TABLEWRAP].type == UDT_Table) {
		OutputDebugStringA("got wrap table\n");
		lua_rawgeti(_currentState, LUA_REGISTRYINDEX, (int)p[ROF_TABLEWRAP].i);
		int t = (int)lua_gettop(_currentState);
		bool have = true;
		while (have) {
			have = false;
			lua_pushnil(_currentState);
			while (lua_next(_currentState, t) != 0) {
				have = true;
				// take value
				lua_pop(_currentState, 1);
				// -1 key
				auto s = lua_tostring(_currentState, -1);
				OutputDebugStringA("delete key ");
				OutputDebugStringA(s);
				OutputDebugStringA("\n");
				lua_pushnil(_currentState);
				lua_settable(_currentState, 1);
				break;
			}
		}
		lua_pop(_currentState, 1);
	}
	*/
}

void RenderObject::__init(lua_State *L) {

	// __setSize();
	lua_pushnil(L);
	auto nilindex = lua_gettop(L);
	_helper_set(L, nilindex, nilindex, ROPROPS - 1);
	lua_pop(L, 1);

	auto p = getData();

	p[ROF_ID].type = UDT_Integer;
	p[ROF_ID].i = renderObjectIterator;
	renderObjectIterator++;

	// p[ROF_PARENT].type = UDT_Nil;

	p[ROF_ISHOVER].type = UDT_Boolean;
	p[ROF_ISHOVER].b = false;

	p[ROF_LPUSHSTATE].type = UDT_Boolean;
	p[ROF_LPUSHSTATE].b = false;
	
	p[ROF_SKIPHOVER].type = UDT_Boolean;
	p[ROF_SKIPHOVER].b = false;

	// p[ROF_CHILDS].type = UDT_Nil;
	// p[ROF_DRAWORDER].type = UDT_Nil;

}

LuaEMethod(RenderObject, draw) {
	CHECK_ARGCLASS(1, DDDrawMachine, _drawMachine);
	__draw(L, _drawMachine);
	return 0;
}

void RenderObject::__drawSelf(DDDrawMachine *drawMachine) { }

void RenderObject::__draw(lua_State *L, DDDrawMachine *drawMachine) {
	
	_lock->lock_shared();
	__drawSelf(drawMachine);

	auto p = getData();

	RenderObject *ro;
	// int table, tp;
	// UserData *ud;

	if (p[ROF_CHILDS].type != UDT_Nil) {
		auto a = (Array *)p[ROF_CHILDS].baseClass;
		a->_lock->lock_shared();

		p = a->getData();
		for (int i = 0, l = a->getCount(); i < l; i++, p++) {
			switch (p->type) {

			case UDT_Class:
				ro = dynamic_cast<RenderObject *>(p->baseClass);
				if (ro) {
					ro->__draw(L, drawMachine);
				}
				break;
			}
		}

		a->_lock->unlock_shared();
	}
		
	_lock->unlock_shared();

}

void RenderObject::__deleteChilds() {
	_lock->lock();

	auto p = getData();
	RenderObject *ro;

	if (p[ROF_CHILDS].type != UDT_Nil) {
		auto a = (Array *)p[ROF_CHILDS].baseClass;
		a->_lock->lock();

		p = a->getData();
		for (int i = 0, l = a->getCount(); i < l; i++, p++) {
			switch (p->type) {
			case UDT_Table:
			case UDT_Function:
				luaL_unref(_currentState, LUA_REGISTRYINDEX, (int)p->i);
				break;

			case UDT_Class:
				ro = dynamic_cast<RenderObject *>(p->baseClass);
				if (ro) {
					ro->__deleteChilds();
				}
				auto share = p->baseClass->share(-1);
				if (share == 0) {
					delete p->baseClass;
				}				
				break;
			}
			p->type = UDT_Nil;
		}

		a->_lock->unlock();
	}

	p = getData();
	for (int i = 0, l = getCount(); i < l; i++, p++) {

		if (p[ROF_TABLEWRAP].type == UDT_Table) {

			// OutputDebugStringA("scan ROF_TABLEWRAP for on del\n");
			lua_rawgeti(_currentState, LUA_REGISTRYINDEX, (int)p[ROF_TABLEWRAP].i);
			lua_pushliteral(_currentState, "onDel");
			lua_gettable(_currentState, -2);
			auto tp = lua_type(_currentState, -1);
			if (tp == LUA_TFUNCTION) {
				// OutputDebugStringA("got method\n");
				lua_insert(_currentState, -2);
				lua_call(_currentState, 1, 0);
			}
			else {
				lua_pop(_currentState, 2);
			}

		}

		switch (p->type) {
		case UDT_Table:
		case UDT_Function:
			luaL_unref(_currentState, LUA_REGISTRYINDEX, (int)p->i);
			break;

		case UDT_Class:
			ro = dynamic_cast<RenderObject *>(p->baseClass);
			if (ro) {
				ro->__deleteChilds();
			}
			auto share = p->baseClass->share(-1);
			if (share == 0) {
				delete p->baseClass;
			}
			break;
		}
		p->type = UDT_Nil;
	}


	

	_lock->unlock();
}

LuaEMethod(RenderObject, del) {

	auto tp = lua_type(L, 1);
	// LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA.
	int valueIndex = 1;

	switch (tp) {

	case LUA_TTABLE: {

		lua_pushliteral(L, "firstChild");
		lua_gettable(L, -2);

		tp = lua_type(L, -1);
		switch (tp) {
		case LUA_TUSERDATA: {
		}
		break;

		default:
			lua_pushfstring(L, "RenderObject:del 2 unsupport type '%s'", lua_typename(L, tp));
			lua_error(L);
		}

		valueIndex = lua_gettop(L);

		
		// need delete all variable
		/*
		bool have = true;
		while (have) {
			have = false;
			lua_pushnil(L);
			while (lua_next(L, 1) != 0) {
				have = true;
				// take value
				lua_pop(L, 1);
				// -1 key
				auto s = lua_tostring(L, -1);
				OutputDebugStringA("delete key ");
				OutputDebugStringA(s);
				OutputDebugStringA("\n");
				lua_pushnil(L);
				lua_settable(L, 1);
				break;
			}
		}
		*/
		
		UserData *ud = (UserData *)lua_touserdata(L, valueIndex);
		RenderObject *ro = dynamic_cast<RenderObject *>((LuaBaseClass *)ud->data);
		_currentState = L;
		ro->__deleteChilds();
		/*
		ro->_lock->lock();
		auto p = ro->getData();
		if (p[ROF_TABLEWRAP].type == UDT_Table) {
			OutputDebugStringA("del got table wrap\n");
			luaL_unref(_currentState, LUA_REGISTRYINDEX, (int)p[ROF_TABLEWRAP].i);
			p[ROF_TABLEWRAP].type = UDT_Nil;
		}
		// ro->_helper_set(L, 1, 1, ROF_TABLEWRAP);
		ro->_lock->unlock();
		*/

	}
	break;

	case LUA_TUSERDATA: {
	}
	break;

	default:
		lua_pushfstring(L, "RenderObject:del 1 unsupport type '%s'", lua_typename(L, tp));
		lua_error(L);

	}

	auto ud = (UserData *)lua_touserdata(L, valueIndex);
	void *d = ud->data;

	_lock->lock_shared();
	// init child list
	auto p = getData();
	if (p[ROF_CHILDS].type == UDT_Class) {
		auto a = (Array *)p[ROF_CHILDS].baseClass;
		a->_lock->lock();

		p = a->getData();
		for (int i = 0, l = a->getCount(); i < l; i++, p++) {
			if (p->type == UDT_Class && d == p->baseClass) {

				RenderObject *ro = dynamic_cast<RenderObject *>(p->baseClass);				
				ro->__deleteChilds();

				auto share = p->baseClass->share(-1);

				// char b[1024];
				// sprintf_s(b, "find deleted child, share %d\n", (int)share);
				// OutputDebugStringA(b);

				if (share == 0) {
					_currentState = L;
					delete p->baseClass;
				}
				p->type = UDT_Nil;
				break;
			}
		}

		a->_lock->unlock();
	}
	_lock->unlock_shared();

	if (valueIndex > 1) {
		lua_pop(L, 1);
	}

	return 0;
}

LuaEMethod(RenderObject, add) {

	auto tp = lua_type(L, 1);
	// LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA.
	int valueIndex;

	switch (tp) {

	case LUA_TTABLE: {
		
		lua_pushliteral(L, "firstChild");
		lua_gettable(L, -2);

		tp = lua_type(L, -1);
		switch (tp) {
		case LUA_TUSERDATA: {
		}
		break;
		default:
			lua_pushfstring(L, "RenderObject:add 2 unsupport type '%s'", lua_typename(L, tp));
			lua_error(L);
		}

		valueIndex = lua_gettop(L);

		// _lock->lock();
		// auto p = getData();

		UserData *ud = (UserData *)lua_touserdata(L, valueIndex);
		RenderObject *ro = dynamic_cast<RenderObject *>((LuaBaseClass *)ud->data);
		ro->_helper_set(L, 1, 1, ROF_TABLEWRAP);

		//_lock->unlock();
	}
	break;

	case LUA_TUSERDATA: {

		auto bc = (LuaBaseClass *)((UserData *)lua_touserdata(L, 1))->data;
		// bc->share(1);
		valueIndex = 1;

	}
	break;

	default:
		lua_pushfstring(L, "RenderObject:add 1 unsupport type '%s'", lua_typename(L, tp));
		lua_error(L);

	}

	_lock->lock_shared();
	// init child list
	auto p = getData();
	if (p[ROF_CHILDS].type == UDT_Nil) {

		_lock->unlock_shared();
		_lock->lock();

		auto childs = LuaClass<Array>::__create(L);
		childs->setmt();
		// childs->share(1);
		p[ROF_CHILDS].type = UDT_Class;
		p[ROF_CHILDS].baseClass = childs;

		_lock->unlock_and_lock_shared();
	}

	auto a = (Array *)p[ROF_CHILDS].baseClass;
	a->_helper_set(L, valueIndex, valueIndex, a->getCount());

	_lock->unlock_shared();

	UserData *ud = (UserData *)lua_touserdata(L, valueIndex);
	RenderObject *ro = dynamic_cast<RenderObject *>((LuaBaseClass *)ud->data);

	ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Class;
	ud->data = this;
	this->registerInNewState(L);
	share(1);

	auto index = lua_gettop(L);
	ro->_execEvent(L, ROF_ONADD, index, index);
	lua_pop(L, 1);

	if (valueIndex > 1) {
		lua_pop(L, 1);
	}


	lua_pushvalue(L, 1);
	return 1;
}

LuaEMethod(RenderObject, event) {
	CHECK_ARG(1, integer);
	int id = (int)lua_tointeger(L, 1);
	// OutputDebugStringA("event start\n");
	_execEvent(L, id, 2, lua_gettop(L));
	// OutputDebugStringA("event end\n");
	return 0;
}

void RenderObject::_execEvent(lua_State *L, int eventIndex, int argsFrom, int argsTo) {

	_lock->lock_shared();
	auto p = getData();
	auto ref = p[eventIndex].i;
	auto r = p[eventIndex].type != UDT_Function;
	_lock->unlock_shared();

	if (r) {		
		return;
	}

	lua_rawgeti(L, LUA_REGISTRYINDEX, (int)ref);
	UserData *ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Class;
	ud->data = this;
	share(1);
	registerInNewState(L);
	int args = 1;
	for (int i = argsFrom; i <= argsTo; i++, args++) {
		lua_pushvalue(L, i);		
	}

	lua_call(L, args, 0);
}

LuaEMethod(RenderObject, cevent) {

	CHECK_ARG(1, integer);
	int id = (int)lua_tointeger(L, 1);
	auto lastId = lua_gettop(L);
	
	_lock->lock_shared();

	auto p = getData();
	RenderObject *ro;
	if (p[ROF_CHILDS].type != UDT_Nil) {
		auto a = (Array *)p[ROF_CHILDS].baseClass;
		a->_lock->lock_shared();

		p = a->getData();
		for (int i = 0, l = a->getCount(); i < l; i++, p++) {
			switch (p->type) {

			case UDT_Class:
				ro = dynamic_cast<RenderObject *>(p->baseClass);
				if (ro) {
					ro->_execEvent(L, id, 2, lastId);					
				}
				break;
			}
		}

		a->_lock->unlock_shared();
	}

	_lock->unlock_shared();
	return 0;

}

bool RenderObject::__isPointOn(long long x, long long y) {

	_lock->lock_shared();
	auto p = getData();

	auto _x = p[ROF_X].i;
	auto _y = p[ROF_Y].i;
	auto _w = p[ROF_W].i;
	auto _h = p[ROF_H].i;

	if (x < _x || y < _y || x >= _x + _w || y >= _y + _h) {
		_lock->unlock_shared();
		return false;
	}

	_lock->unlock_shared();
	return true;
}

RenderObject *RenderObject::__findHoveredObject(int x, int y) {

	bool state = true, self = false;

	_lock->lock_shared();
	auto p = getData();
	/*
	if (p[ROF_SKIPHOVER].b) {
		_lock->unlock_shared();
		return 0;
	}
	*/

	auto r = p[ROF_X].type != UDT_Nil;
	
	_lock->unlock_shared();

	if (r) {
		if (!p[ROF_SKIPHOVER].b) {
			state = __isPointOn(x, y);
			if (state) {
				self = true;
			}
		}
	}

	if (!state) {
		return 0;
	}

	_lock->lock_shared();
	RenderObject *ro;
	if (p[ROF_CHILDS].type != UDT_Nil) {
		auto a = (Array *)p[ROF_CHILDS].baseClass;
		_lock->unlock_shared();
		a->_lock->lock_shared();

		p = a->getData();
		for (int i = 0, l = a->getCount(); i < l; i++, p++) {
			switch (p->type) {
			case UDT_Class:
				ro = dynamic_cast<RenderObject *>(p->baseClass);
				if (ro) {
					ro = ro->__findHoveredObject(x, y);
					if (ro) {
						a->_lock->unlock_shared();
						return ro;
					}
				}
				break;
			}
		}

		a->_lock->unlock_shared();
	}
	else {
		_lock->unlock_shared();
	}

	if (self) {
		return this;
	}

	return 0;
}


LuaEMethod(RenderObject, isPointOn) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);

	auto x = lua_tointeger(L, 1);
	auto y = lua_tointeger(L, 2);

	auto ro = __findHoveredObject((int)x, (int)y);

	if (ro) {
		auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
		ud->type = UDT_Class;
		ud->data = ro;
		ro->share(1);
		ro->registerInNewState(L);
	}
	else {
		lua_pushnil(L);
	}

	return 1;
}

LuaAMethods(RenderObject, [](lua_State *L, int methods) { LuaAddMethods(Array); }) = {
	LuaMethodDesc(RenderObject, isPointOn),
	LuaMethodDesc(RenderObject, event),
	LuaMethodDesc(RenderObject, cevent),
	LuaMethodDesc(RenderObject, add),
	LuaMethodDesc(RenderObject, del),
	LuaMethodDesc(RenderObject, draw),
	{ 0,0 }
};
 
LuaMetaMethods(RenderObject) = {
	{ 0,0 }
};

void module_render_object(lua_State *L) {
	LuaClass<RenderObject>::Register(L);
}
