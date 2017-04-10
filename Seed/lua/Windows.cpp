#include ".\Windows.h"


wchar_t *char2tchar(const char *s) {
	size_t size = strlen(s) + 1;
	wchar_t *wbuf = new wchar_t[size];
	size_t outSize;
	mbstowcs_s(&outSize, wbuf, size, s, size - 1);
	return wbuf;
}

char *tchar2char(const wchar_t *t) {
	auto len = wcslen(t) + 1;
	auto s = new char[len];
	size_t sz;
	wcstombs_s(&sz, s, len, t, len);
	return s;
}


LRESULT CALLBACK _WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

LuaClass(WindowClass) 

	WindowClass(unsigned short int classId) : LuaBaseClass(classId), _style(0), _instance(0), _className(NULL) {
	}

	~WindowClass() {
		if (_className) {
			delete _className;
			_className = NULL;
		}
	}

	
	LuaMethod(WindowClass, __newindex) {

		CHECK_ARG(1, string);
		auto field = lua_tostring(L, 1);

		if (strcmp(field, "style") == 0) {
			CHECK_ARG(2, integer);
			_style = (UINT)lua_tointeger(L, 2);
		}
		else if (strcmp(field, "instance") == 0) {
			CHECK_ARG(2, integer);
			_instance = lua_tointeger(L, 2);
		}
		else if (strcmp(field, "lpszClassName") == 0) {
			CHECK_ARG(2, string);
			size_t sz;
			auto s = lua_tolstring(L, 2, &sz);
			_className = new char[sz + 1];
			memcpy(_className, s, sz + 1);
		}
		else {
			luaL_where(L, 1);
			lua_pushfstring(L, "%s wrong field '%s'", lua_tostring(L, -1), lua_tostring(L, 1));
			lua_remove(L, -2);
			lua_error(L);
		}
		
		return 0;
	}
	
	LuaMethod(WindowClass, _register) {

		WNDCLASSEXW wcex;
		memset(&wcex, 0, sizeof(wcex));
		wcex.cbSize = sizeof(wcex);

		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

		wcex.style = _style;
		wcex.hInstance = (HINSTANCE)_instance;

		auto c = char2tchar(_className);
		wcex.lpszClassName = c;

		wcex.lpfnWndProc = _WindowProc;
		wcex.cbWndExtra = sizeof(void *) * 16;

		auto rs = RegisterClassExW(&wcex);

		delete c;
		// cleanup
		if (_className) {
			delete _className;
			_className = NULL;
		}

		if(rs == 0) {
			lua_pushfstring(L, "WindowClass.register fail");
			lua_error(L);
		}

		return 0;
	}
private:
	UINT _style;
	long long _instance;
	char *_className;
};

LuaMethods(WindowClass) = {
	LuaMethodDesc_(WindowClass, register),
	{ 0,0 }
};

LuaMetaMethods(WindowClass) = {
	// LuaMethodDesc(Array, __tostring),
	LuaMethodDesc(WindowClass, __newindex),
	{ 0,0 }
};


LuaEMethod(Window, close) {
	_close();
	return 0;
}

LuaEMethod(Window, __newindex) {

	CHECK_ARG(1, string);
	auto field = lua_tostring(L, 1);

	if (strcmp(field, "instance") == 0) {
		CHECK_ARG(2, integer);
		_hinstance = (HINSTANCE)lua_tointeger(L, 2);
	}
	else if (strcmp(field, "className") == 0) {
		CHECK_ARG(2, string);
		size_t sz;
		auto s = lua_tolstring(L, 2, &sz);
		_className = new char[sz + 1];
		memcpy(_className, s, sz + 1);
	}
	else if (strcmp(field, "title") == 0) {
		CHECK_ARG(2, string);
		size_t sz;
		auto s = lua_tolstring(L, 2, &sz);
		_windowName = new char[sz + 1];
		memcpy(_windowName, s, sz + 1);
	}
	else if (strcmp(field, "wndProc") == 0) {
		CHECK_ARG(2, function);
		lua_insert(L, 2);
		_wndProc = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	else {
		luaL_where(L, 1);
		lua_pushfstring(L, "%s wrong field '%s'", lua_tostring(L, -1), lua_tostring(L, 1));
		lua_remove(L, -2);
		lua_error(L);
	}

	return 0;
}

LuaEMethod(Window, trackMouseLeave) {
	TRACKMOUSEEVENT tme = { sizeof(tme) };
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = _hwnd;
	TrackMouseEvent(&tme);
	return 0;
}
 
LuaEMethod(Window, create) {

	auto c = char2tchar(_className), t = char2tchar(_windowName);
	_windowPos.length = 0;

	_L = L;

	_hwnd = CreateWindowW(
		c,
		t,
		_style,
		_x,
		_y,
		_width,
		_height,
		0,
		0,
		_hinstance,
		0
	);
	delete t;
	delete c;
			
	if (_hwnd) {
		lua_pushboolean(L, true);			
		SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)this);

		HMENU hMenu = GetSystemMenu(_hwnd, FALSE);
		DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
	}
	else {
		lua_pushboolean(L, false);
	}

	_cleanup();

	return 1;
}

LuaEMethod(Window, show) {
	CHECK_ARG(1, integer);
	ShowWindow(_hwnd, (int)lua_tointeger(L, 1));
	return 0;
}

LuaEMethod(Window, update) {
	UpdateWindow(_hwnd);
	return 0;
}

LuaEMethod(Window, title) {
	lua_pushstring(L, _windowName);
	return 1;
}

LuaEMethod(Window, removeStyle) {

	CHECK_ARG(1, integer);
	auto _removeStyle = lua_tointeger(L, 1);
	auto dw = (DWORD)_removeStyle;

	DWORD _windowStyle = GetWindowLong(_hwnd, GWL_STYLE);

	SetWindowLong(_hwnd, GWL_STYLE, _windowStyle & ~dw);

	return 0;
}

LuaEMethod(Window, addStyle) {
	CHECK_ARG(1, integer);
	auto _removeStyle = lua_tointeger(L, 1);
	auto dw = (DWORD)_removeStyle;

	DWORD _windowStyle = GetWindowLong(_hwnd, GWL_STYLE);

	SetWindowLong(_hwnd, GWL_STYLE, _windowStyle | dw);
	return 0;
}

LuaEMethod(Window, setPlacement) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	CHECK_ARG(3, integer);
	CHECK_ARG(4, integer);

	long x = (long)lua_tointeger(L, 1);
	long y = (long)lua_tointeger(L, 2);
	long w = (long)lua_tointeger(L, 3);
	long h = (long)lua_tointeger(L, 4);

	WINDOWPLACEMENT _windowPos = { sizeof(_windowPos) };
	_windowPos.showCmd = SW_NORMAL;
	_windowPos.rcNormalPosition.left = x;
	_windowPos.rcNormalPosition.top = y;
	_windowPos.rcNormalPosition.right = x + w;
	_windowPos.rcNormalPosition.bottom = y + h;

	SetWindowPlacement(_hwnd, &_windowPos);
	SetWindowPos(_hwnd, NULL,
		_windowPos.rcNormalPosition.left, _windowPos.rcNormalPosition.top, _windowPos.rcNormalPosition.right, _windowPos.rcNormalPosition.bottom,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

	return 0;
}

LuaEMethod(Window, getPlacement) {

	WINDOWPLACEMENT _windowPos = { sizeof(_windowPos) };
	GetWindowPlacement(_hwnd, &_windowPos);

	RECT rect;
	GetClientRect(_hwnd, &rect);
	auto _width = rect.right - rect.left;
	auto _height = rect.bottom - rect.top;

	lua_pushinteger(L, _windowPos.rcNormalPosition.left);
	lua_pushinteger(L, _windowPos.rcNormalPosition.top);

	lua_pushinteger(L, _width);
	lua_pushinteger(L, _height);

	return 4;
}


LuaEMethod(Window, savePlacement) {

	_windowPos = { sizeof(_windowPos) };
	GetWindowPlacement(_hwnd, &_windowPos);

	return 0;
}

LuaEMethod(Window, restorePlacement) {

	if (_windowPos.length == 0) {
		return 0;
	}

	SetWindowPlacement(_hwnd, &_windowPos);
	SetWindowPos(_hwnd, NULL,
		_windowPos.rcNormalPosition.left, _windowPos.rcNormalPosition.top, _windowPos.rcNormalPosition.right, _windowPos.rcNormalPosition.bottom,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);


	return 0;
}


LuaEMethod(Window, maximizePos) {

	WINDOWPLACEMENT __windowPos;
	__windowPos.length = sizeof(__windowPos);
	__windowPos.showCmd = SW_NORMAL;
	__windowPos.rcNormalPosition.left = 0;
	__windowPos.rcNormalPosition.top = 0;
	__windowPos.rcNormalPosition.right = 200;
	__windowPos.rcNormalPosition.bottom = 200;

	SetWindowPlacement(_hwnd, &__windowPos);
	SetWindowPos(_hwnd, NULL,
		__windowPos.rcNormalPosition.left, __windowPos.rcNormalPosition.top, __windowPos.rcNormalPosition.right, __windowPos.rcNormalPosition.bottom,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

	return 0;
}

LuaEMethod(Window, validateRect) {
	ValidateRect(_hwnd, NULL);
	return 0;
}


LRESULT Window::wndProc(UINT message, LPARAM lParam, WPARAM wParam, bool *error, char **errorString) {

	auto sv = lua_gettop(_L);

	lua_pushcfunction(_L, errorHandler);

	lua_rawgeti(_L, LUA_REGISTRYINDEX, _wndProc);

	lua_pushinteger(_L, (long long)_hwnd);
	lua_pushinteger(_L, message);
	lua_pushinteger(_L, lParam);
	lua_pushinteger(_L, wParam);

	LRESULT r = 0;

	if (lua_pcall(_L, 4, 1, -6)) {
			
		*error = true;
		*errorString = (char *)lua_tostring(_L, -1);
	}
	else {
			
		r = (LRESULT)lua_tointeger(_L, -1);
		*error = false;
	}

	lua_settop(_L, sv);

	return r;
}

LuaMethods(Window) = {
	LuaMethodDesc(Window, create),
	LuaMethodDesc(Window, close),
	LuaMethodDesc(Window, show),
	LuaMethodDesc(Window, update),
	LuaMethodDesc(Window, removeStyle),
	LuaMethodDesc(Window, addStyle),
	LuaMethodDesc(Window, restorePlacement),
	LuaMethodDesc(Window, savePlacement),
	LuaMethodDesc(Window, getPlacement),
	LuaMethodDesc(Window, setPlacement),
	LuaMethodDesc(Window, validateRect),
	LuaMethodDesc(Window, maximizePos),
	LuaMethodDesc(Window, trackMouseLeave),
	
	// props
	LuaMethodDesc(Window, title),
	{ 0,0 }
};

LuaMetaMethods(Window) = {
	// LuaMethodDesc(Array, __tostring),
	LuaMethodDesc(Window, __newindex),
	{ 0,0 }
};

static int luaC_MessageLoop(lua_State *L) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, function);

	auto iterval = lua_tointeger(L, 1);
	MSG msg;
	bool work = true;
	while (work) {

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			switch (msg.message) {
			case WM_QUIT:
				work = false;
				break;
			}
			DispatchMessage(&msg);
		}

		lua_pushvalue(L, 2);
		lua_call(L, 0, 0);

		Sleep((DWORD)iterval);
	}

	lua_pushinteger(L, (int)msg.wParam);

	return 1;
}

static int luaC_DefWindowProc(lua_State *L) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	CHECK_ARG(3, integer);
	CHECK_ARG(4, integer);

	HWND hWnd = (HWND)lua_tointeger(L, 1);
	UINT message = (UINT)lua_tointeger(L, 2);
	LPARAM lParam = lua_tointeger(L, 3);
	WPARAM wParam = lua_tointeger(L, 4);

	lua_pushinteger(L, DefWindowProc(hWnd, message, wParam, lParam));

	return 1;
}


static int luaC_PostQuitMessage(lua_State *L) {

	CHECK_ARG(1, integer);

	PostQuitMessage((int)lua_tointeger(L, 1));
	return 0;
}


LRESULT CALLBACK _WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	auto ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (ptr) {
		auto w = (Window *)ptr;
		bool error;
		char *errorString;

		auto l = w->wndProc(message, lParam, wParam, &error, &errorString);

		if (error) {
			OutputDebugStringA("error in wndProc\n");
			OutputDebugStringA(errorString);
			OutputDebugStringA("\n");
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return l;
	}
	else {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

Font::Font(unsigned short int classId) : LuaBaseClass(classId), _font(0) { }

void Font::__init(lua_State *L) {
	CHECK_ARG(1, string);
	CHECK_ARG(2, integer);

	_name = lua_tostring(L, 1);
	_size = (int)lua_tointeger(L, 2);
}

Font::~Font() {
}


LuaEMethod(Font, __newindex) {

	CHECK_ARG(1, string);
	auto field = lua_tostring(L, 1);

	/*
	if (strcmp(field, "style") == 0) {
		CHECK_ARG(2, integer);
		_style = (UINT)lua_tointeger(L, 2);
	}
	else if (strcmp(field, "instance") == 0) {
		CHECK_ARG(2, integer);
		_instance = lua_tointeger(L, 2);
	}
	else if (strcmp(field, "lpszClassName") == 0) {
		CHECK_ARG(2, string);
		size_t sz;
		auto s = lua_tolstring(L, 2, &sz);
		_className = new char[sz + 1];
		memcpy(_className, s, sz + 1);
	}
	else {
		luaL_where(L, 1);
		lua_pushfstring(L, "%s wrong field '%s'", lua_tostring(L, -1), lua_tostring(L, 1));
		lua_remove(L, -2);
		lua_error(L);
	}
	*/

	return 0;
}

LuaEMethod(Font, create) {

	LOGFONT MyLogFont;

	auto wname = char2tchar(_name.c_str());
	ZeroMemory(&MyLogFont, sizeof(MyLogFont));
	wcscpy_s(MyLogFont.lfFaceName, wname);
	delete wname;

	MyLogFont.lfQuality = ANTIALIASED_QUALITY;
	MyLogFont.lfHeight = _size;

	MyLogFont.lfWeight = FW_NORMAL;
	// MyLogFont.lfWeight = FW_BOLD;

	MyLogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;

	_font = CreateFontIndirect(&MyLogFont);

	return 0;
}


LuaMethods(Font) = {
	LuaMethodDesc(Font, create),
	{ 0,0 }
};

LuaMetaMethods(Font) = {
	// LuaMethodDesc(Array, __tostring),
	LuaMethodDesc(Font, __newindex),
	{ 0,0 }
};

Cursor::Cursor(unsigned short int classId) : LuaBaseClass(classId), _cursor(0) { }

void Cursor::__init(lua_State *L) {
	CHECK_ARG(1, integer);

	auto r = MAKEINTRESOURCE(lua_tointeger(L, 1));
	_cursor = LoadCursor(NULL, r);
}

Cursor::~Cursor() {
}

LuaEMethod(Cursor, set) {
	if (_cursor) {
		SetCursor(_cursor);
	}
	return 0;
}

LuaMethods(Cursor) = {
	LuaMethodDesc(Cursor, set),
	{ 0,0 }
};

LuaMetaMethods(Cursor) = {
	{ 0,0 }
};


void module_windows(lua_State *L) {

	LuaClass<WindowClass>::Register(L);
	LuaClass<Window>::Register(L);
	LuaClass<Font>::Register(L);
	LuaClass<Cursor>::Register(L);

	lua_register(L, "MessageLoop", luaC_MessageLoop);
	lua_register(L, "DefWindowProc", luaC_DefWindowProc);
	lua_register(L, "PostQuitMessage", luaC_PostQuitMessage);
}
