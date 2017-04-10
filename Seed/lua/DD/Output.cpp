#include "Output.h"
#include "DD.h"

DDOutput::DDOutput(unsigned short int classId) : LuaBaseClass(classId) { }

void DDOutput::__init(lua_State *L) {
	
	CHECK_ARG(1, integer);
	_type = (int)lua_tointeger(L, 1);
	_dds = NULL;

	switch (_type) {

	case OT_Windowed: {
		CHECK_ARGCLASS(2, Window, wnd);
		_window = wnd;
		_backSurfaceCount = 2;
		_backSurfaceIndex = 0;
		_dds_Primary = NULL;
		_ddc_Clipper = NULL;

		_dds_Back = new LPDIRECTDRAWSURFACE7[_backSurfaceCount];
		for (int i = 0; i < _backSurfaceCount; i++) {
			_dds_Back[i] = NULL;
		}

	}
	break;

	case OT_Fullscreen: {
		_dds_backFullScreen = NULL;
		_dds_Primary = NULL;
	}
	break;

	}

}

LuaEMethod(DDOutput, create) {

	CHECK_ARGCLASS(1, DD, _ddc);
	auto _dd = _ddc->_getDD();

	DDSURFACEDESC2 ddsd;
	DDSURFACEDESC2 _curmonitorModeInfo;
	HRESULT hr;

	switch (_type) {
	case OT_Windowed:

		RECT rect;
		GetClientRect(_window->_getHWND(), &rect);
		_width = rect.right - rect.left;
		_height = rect.bottom - rect.top;

		if (_height < 1 || _width < 1) {
			return 0;
		}

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		hr = _dd->CreateSurface(&ddsd, &_dds_Primary, NULL);
		if (FAILED(hr)) {
			lua_pushfstring(L, "Error createSurfaces: %s", DDErrorString(hr));
			lua_error(L);
		}

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth = _width;
		ddsd.dwHeight = _height;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

		_backSurfaceIndex = 0;
		for (int i = 0; i < _backSurfaceCount; i++) {

			hr = _dd->CreateSurface(&ddsd, &_dds_Back[i], NULL);
			if (FAILED(hr)) {
				lua_pushfstring(L, "Error createBackSurface: %s", DDErrorString(hr));
				lua_error(L);
			}

		}

		_dds = _dds_Back[_backSurfaceIndex];

		hr = _dd->CreateClipper(0, &_ddc_Clipper, NULL);
		if (FAILED(hr)) {
			lua_pushfstring(L, "Error CreateClipper: %s", DDErrorString(hr));
			lua_error(L);
		}

		hr = _ddc_Clipper->SetHWnd(0, _window->_getHWND());
		if (FAILED(hr)) {
			lua_pushfstring(L, "Error SetHWnd: %s", DDErrorString(hr));
			lua_error(L);
		}

		hr = _dds_Primary->SetClipper(_ddc_Clipper);
		if (FAILED(hr)) {
			lua_pushfstring(L, "Error SetClipper: %s", DDErrorString(hr));
			lua_error(L);
		}

		break;

	case OT_Fullscreen:

		memset(&_curmonitorModeInfo, 0, sizeof(_curmonitorModeInfo));
		_curmonitorModeInfo.dwSize = sizeof(_curmonitorModeInfo);
		hr = _dd->GetDisplayMode(&_curmonitorModeInfo);
		if (FAILED(hr)) {
			lua_pushfstring(L, "Error GetDisplayMode: %s", DDErrorString(hr));
			lua_error(L);
		}

		_width = _curmonitorModeInfo.dwWidth;
		_height = _curmonitorModeInfo.dwHeight;

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
		ddsd.dwBackBufferCount = 2;

		hr = _dd->CreateSurface(&ddsd, &_dds_Primary, NULL);
		if (FAILED(hr)) {
			lua_pushfstring(L, "Error CreateSurface: %s", DDErrorString(hr));
			lua_error(L);
		}

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth = _width;
		ddsd.dwHeight = _height;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

		hr = _dd->CreateSurface(&ddsd, &_dds_backFullScreen, NULL);
		if (FAILED(hr)) {
			lua_pushfstring(L, "Error createBackSurface: %s", DDErrorString(hr));
			lua_error(L);
		}

		_dds = _dds_backFullScreen;

		break;
	}

	


	return 0;
}

LuaEMethod(DDOutput, release) {

	switch (_type) {

	case OT_Windowed: {

		if (_ddc_Clipper) {
			_ddc_Clipper->Release();
			_ddc_Clipper = NULL;
		}

		if (_dds_Primary) {
			_dds_Primary->Release();
			_dds_Primary = NULL;
		}

		for (int i = 0; i < _backSurfaceCount; i++) {
			if (_dds_Back[i]) {
				_dds_Back[i]->Release();
				_dds_Back[i] = NULL;
			}
		}
	}
	break;

	case OT_Fullscreen: {

		if (_dds_backFullScreen) {
			_dds_backFullScreen->Release();
			_dds_backFullScreen = NULL;
		}

		if (_dds_Primary) {
			_dds_Primary->Release();
			_dds_Primary = NULL;
		}

	}
	break;

	}

	return 0;
}

LuaEMethod(DDOutput, getSize) {
	if (_dds_Primary) {
		lua_pushinteger(L, _width);
		lua_pushinteger(L, _height);
	}
	else {
		lua_pushinteger(L, 0);
		lua_pushinteger(L, 0);
	}
	return 2;
}
LuaEMethod(DDOutput, isValid) {
	lua_pushboolean(L, _dds_Primary != NULL);
	return 1;
}
	
void DDOutput::__checkSurfaces() {

	switch (_type) {

	case OT_Windowed: {

		if (_dds_Primary && _dds_Primary->IsLost() == DDERR_SURFACELOST)
			_dds_Primary->Restore();

		if (_dds_Back && _dds_Back[_backSurfaceIndex] && _dds_Back[_backSurfaceIndex]->IsLost() == DDERR_SURFACELOST)
			_dds_Back[_backSurfaceIndex]->Restore();
	}
	break;

	case OT_Fullscreen: {

		if (_dds_Primary && _dds_Primary->IsLost() == DDERR_SURFACELOST)
			_dds_Primary->Restore();

		if (_dds_backFullScreen && _dds_backFullScreen->IsLost() == DDERR_SURFACELOST)
			_dds_backFullScreen->Restore();
	}
	break;

	}

}

LuaEMethod(DDOutput, flip) {

	switch (_type) {

	case OT_Windowed: {

		if (_dds_Primary == NULL) {
			return 0;
		}

		RECT    rcSrc;
		RECT    rcDest;
		POINT   p;

		p.x = 0; p.y = 0;
		::ClientToScreen(_window->_getHWND(), &p);

		SetRect(&rcDest, 0, 0, _width, _height);
		OffsetRect(&rcDest, p.x, p.y);

		SetRect(&rcSrc, 0, 0, _width, _height);

		auto hr = _dds_Primary->Blt(&rcDest, _dds_Back[_backSurfaceIndex], &rcSrc, DDBLT_WAIT, NULL);

		_backSurfaceIndex++;
		if (_backSurfaceIndex >= _backSurfaceCount) {
			_backSurfaceIndex = 0;
		}

		_dds = _dds_Back[_backSurfaceIndex];
	}
	break;

	case OT_Fullscreen: {

		if (_dds_Primary->IsLost() == DDERR_SURFACELOST) return 0;
		if (_dds_backFullScreen->IsLost() == DDERR_SURFACELOST) return 0;		

		DDSCAPS2 surfcaps;
		memset(&surfcaps, 0, sizeof(surfcaps));
		surfcaps.dwCaps = DDSCAPS_BACKBUFFER;

		auto b = _dds_Primary;
		HRESULT hr = _dds_Primary->GetAttachedSurface(&surfcaps, &b);
		if (hr != DD_OK) {
			_dds_backFullScreen = NULL;
			lua_pushfstring(L, "Error GetAttachedSurface: %s", DDErrorString(hr));
			lua_error(L);
		}

		DDSURFACEDESC2 ddsd;
		ddsd.dwSize = sizeof(ddsd);
		hr = b->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		if (hr != DD_OK) {
			lua_pushfstring(L, "Error b->Lock: %s", DDErrorString(hr));
			lua_error(L);
		}

		DDSURFACEDESC2 ddsdb;
		ddsdb.dwSize = sizeof(ddsdb);
		hr = _dds_backFullScreen->Lock(NULL, &ddsdb, DDLOCK_WAIT, NULL);
		if (hr != DD_OK) {
			lua_pushfstring(L, "Error bs->Lock: %s", DDErrorString(hr));
			lua_error(L);
		}

		char *s = (char *)ddsdb.lpSurface;
		char *d = (char *)ddsd.lpSurface;
		auto sl = ddsdb.lPitch;
		auto dl = ddsd.lPitch;

		
		for (int y = 0; y < _height; y++) {
			memcpy(d, s, _width * 4);
			s += sl;
			d += dl;
		}

		b->Unlock(NULL);
		_dds_backFullScreen->Unlock(NULL);

		hr = _dds_Primary->Flip(NULL, DDFLIP_WAIT);
		if (hr != DD_OK) {
			lua_pushfstring(L, "Error _dds_Primary->Flip: %s", DDErrorString(hr));
			lua_error(L);
		}
	}
	break;

	}

	return 0;
}


LuaMethods(DDOutput) = {
	LuaMethodDesc(DDOutput, create),
	LuaMethodDesc(DDOutput, release),
	LuaMethodDesc(DDOutput, flip),
	LuaMethodDesc(DDOutput, isValid),
	LuaMethodDesc(DDOutput, getSize),	
	{ 0,0 }
};

LuaMetaMethods(DDOutput) = {
	{ 0,0 }
};


void module_dd_output(lua_State *L) {
	LuaClass<DDOutput>::Register(L);
}