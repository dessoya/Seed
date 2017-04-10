#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "..\LuaScript.h"
#include "..\LuaBaseClass.h"
#include "..\LuaClass.h"
#include "..\LuaCheck.h"

wchar_t *char2tchar(const char *s);
char *tchar2char(const wchar_t *t);

	
LuaClass(Window)

	Window(unsigned short int classId) : LuaBaseClass(classId), _className(0), _windowName(0),
		_width(CW_USEDEFAULT), _height(CW_USEDEFAULT), _x(CW_USEDEFAULT), _y(CW_USEDEFAULT),
		_style(WS_OVERLAPPED | WS_THICKFRAME  | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX), _hinstance(0), _wndProc(LUA_NOREF) {
	}

	~Window() {
		// OutputDebugStringA("~Window\n");
		if (_hwnd) {
			_close();
		}
		_cleanup();
		if (_windowName) {
			delete _windowName;
			_windowName = 0;
		}
	}
	
	void _close() {
		SetWindowLongPtr(_hwnd, GWLP_USERDATA, 0);
		CloseWindow(_hwnd);
		_hwnd = 0;
	}

	const char *_getTitle() { return _windowName; }
	HWND _getHWND() { return _hwnd; }

	LRESULT wndProc(UINT message, LPARAM lParam, WPARAM wParam, bool *error, char **errorString);

	LuaIMethod(close);
	LuaIMethod(__newindex);
	LuaIMethod(create);
	LuaIMethod(show);
	LuaIMethod(update);
	LuaIMethod(title);
	LuaIMethod(removeStyle);
	LuaIMethod(addStyle);

	LuaIMethod(restorePlacement);
	LuaIMethod(savePlacement);
	LuaIMethod(setPlacement);
	LuaIMethod(getPlacement);

	LuaIMethod(validateRect);
	LuaIMethod(maximizePos);
	LuaIMethod(trackMouseLeave);
	
private:

	void _cleanup() {
		if (_className) {
			delete _className;
			_className = 0;
		}
	}

	int _width, _height, _x, _y, _style, _wndProc;
	HINSTANCE _hinstance;
	HWND _hwnd;
	WINDOWPLACEMENT _windowPos;
	char *_className, *_windowName;
	lua_State* _L;
};


LuaClass(Font)

	Font(unsigned short int classId);
	~Font();

	virtual void __init(lua_State *L);

	LuaIMethod(__newindex);
	LuaIMethod(create);

	HFONT _font;

private:
	std::string _name;
	int _size;

};


LuaClass(Cursor)

	Cursor(unsigned short int classId);
	~Cursor();

	void __init(lua_State *L);
	LuaIMethod(set);


	HCURSOR _cursor;
};
