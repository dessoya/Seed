#include "..\..\LuaScript.h"
#include "..\..\LuaTypes.h"
#include "..\..\LuaBaseClass.h"
#include "..\..\LuaClass.h"
#include "..\..\LuaCheck.h"
#include "..\Windows.h"
#include "DD.h"

LuaClass(DDRenderer)

	DDRenderer(lua_State *L) : LuaBaseClass() {

		CHECK_ARGCLASS(1, DD, dd);
		CHECK_ARGCLASS(2, Window, wnd);

		_window = wnd;
		_dd = dd;

		// ARGCLASS(1, DD, dd);

		// dd->_getDD();

		/*
		auto ud = (UserData *)lua_touserdata(L, 1);
		auto w = (Window *)ud->data;
		w->_getTitle();
		*/

	}

	~DDRenderer() {
		OutputDebugStringA("~Renderer\n");
	}

	LuaMethod(DDRenderer, enableWindowedMode) {

		auto hr = _dd->_getDD()->SetCooperativeLevel(_window->_getHWND(), DDSCL_NORMAL | DDSCL_MULTITHREADED);
		if (FAILED(hr)) {
			lua_pushstring(L, DDErrorString(hr));
			lua_error(L);
		}

		return 0;
	}
	
private:

	Window *_window;
	DD *_dd;

};

LuaMethods(DDRenderer) = {
	LuaMethodDesc(DDRenderer, enableWindowedMode),
	{ 0,0 }
};

LuaMetaMethods(DDRenderer) = {
	// LuaMethodDesc(Queue, __tostring),
	// LuaMethodDesc(Queue, __newindex),
	{ 0,0 }
};

/*
static int luaC_threadSetName(lua_State *L) {

CHECK_ARG(1, string);

// Logger::setThreadName(lua_tostring(L, 1));

return 0;
}
*/



void module_dd_renderer(lua_State *L) {
	LuaClass<DDRenderer>::Register(L);

	// lua_register(L, "threadSetName", luaC_threadSetName);
}