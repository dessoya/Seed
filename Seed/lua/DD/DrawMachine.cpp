#include "DrawMachine.h"
#include "Output.h"
#include "mmintrin.h"
#include "DD.h"

DDDrawMachine::DDDrawMachine(unsigned short int classId) : LuaBaseClass(classId), _output(0) {
}

void DDDrawMachine::__lock() {

	DDSURFACEDESC2 DDSDesc;
	DDSDesc.dwSize = sizeof(DDSDesc);

	HRESULT hr = _output->_dds->Lock(NULL, &DDSDesc, DDLOCK_WAIT, NULL);
	if (hr != DD_OK) {
		// lua_pushfstring("Error DrawMachine::__lock %sn")
		char b[1024];
		sprintf_s(b, "Error DrawMachine::__lock %s\n" ,DDErrorString(hr));
		OutputDebugStringA(b);
		exit(1);
	}

	_surface = (unsigned char *)DDSDesc.lpSurface;
	_pitch = DDSDesc.lPitch;
}

void DDDrawMachine::__unlock() {
	_output->_dds->Unlock(NULL);
}

void DDDrawMachine::__drawRect_pure(int x, int y, int w, int h, DWORD color) {

	float c[8];
	DWORD *cc = (DWORD *)c;
	cc[0] = cc[1] = cc[2] = cc[3] = color;
	cc[4] = cc[5] = cc[6] = cc[7] = color; // 0x114455;

	// __m128 m  = _mm_loadu_ps(c);
	// __m128d m = _mm_loadu_pd((double *)c);
	// __m256d m = _mm256_loadu_pd((double *)c);
	__m256i m = _mm256_loadu_si256((__m256i *)cc);
	// _mm256_store_pd

	unsigned char *line = &_surface[ y * _pitch + x * 4 ];

	int pre = x & 7;
	w -= pre;

	int i = w >> 5;
	int o = w & 0x1f;
	

	for (int y1 = y, y2 = y + h; y1 < y2; y1++) {

		// 4 pixel and 16 bytes

		auto _line = line;

		// 16 * 4 = 64 bytes
		// 16 pixel

		int o2 = pre;
		while (o2--) {
			*(DWORD *)_line = color;
			_line += 4;
		}

		for (int j = 0; j < i; j++) {

			_mm256_store_si256((__m256i *)(_line + 32 * 0), m);
			_mm256_store_si256((__m256i *)(_line + 32 * 1), m);
			_mm256_store_si256((__m256i *)(_line + 32 * 2), m);
			_mm256_store_si256((__m256i *)(_line + 32 * 3), m);
			/*
			_mm256_store_si256((__m256i *)(_line + 32 * 4), m);
			_mm256_store_si256((__m256i *)(_line + 32 * 5), m);
			_mm256_store_si256((__m256i *)(_line + 32 * 6), m);
			_mm256_store_si256((__m256i *)(_line + 32 * 7), m);
			*/

			/*
			_mm256_store_pd((double *)(_line + 32 * 1), m);
			_mm256_store_pd((double *)(_line + 32 * 2), m);
			_mm256_store_pd((double *)(_line + 32 * 3), m);
			_mm256_store_pd((double *)(_line + 32 * 4), m);
			_mm256_store_pd((double *)(_line + 32 * 5), m);
			_mm256_store_pd((double *)(_line + 32 * 6), m);
			_mm256_store_pd((double *)(_line + 32 * 7), m);
			*/
			// memset(_line, 0, 32 * 8);
			_line += 32 * 4;
			// _line += 16;
		}

		o2 = o;
		while (o2--) {
			*(DWORD *)_line = color;
			_line += 4;
		}

		line += _pitch;
	}
	

}

void DDDrawMachine::__drawRect(int x, int y, int w, int h, DWORD color) {

	if (x > _output->_width || y > _output->_height || w < 1 || h < 1) return;

	DDBLTFX ddbfx;
	RECT rcDest;

	ddbfx.dwSize = sizeof(ddbfx);
	ddbfx.dwFillColor = color;

	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}

	int xe = x + w;
	if (xe > _output->_width) {
		xe = _output->_width;
	}

	int ye = y + h;
	if (ye > _output->_height) {
		ye = _output->_height;
	}

	SetRect(&rcDest, x, y, xe, ye);
	_output->_dds->Blt(&rcDest, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ddbfx);
}

void DDDrawMachine::__drawText(int x, int y, const char *text, HFONT font, DWORD color) {
	HDC hdc;	
	_output->_dds->GetDC(&hdc);
	if (hdc == NULL) {
		return;
	}

	SelectObject(hdc, font);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, color);

	TextOutA(hdc, x, y, text, (int)strlen(text));

	_output->_dds->ReleaseDC(hdc);
}

void DDDrawMachine::__drawImage(int x, int y, Image *image, int fromx, int fromy, int w, int h) {

	if (y >= _output->_height || x >= _output->_width || x + w < 0 || y + h < 0) { return; }

	if (y + h > _output->_height) { h = _output->_height - y; }
	if (x + w > _output->_width) { w = _output->_width - x; }

	if (y < 0) {
		h += y;
		fromy -= y;
		y = 0;
	}

	if (x < 0) {
		w += x;
		fromx -= x;
		x = 0;
	}

	__lock();

	unsigned char *p = _surface;
	unsigned char *s = (unsigned char *)image->_data;

	long l = _pitch;
	long ls = image->_width * 4;

	unsigned char *d = &p[l * y];
	s = &s[fromy * ls];

	for (int yi = 0; yi < h; yi++) {
		unsigned char *dl = &d[x * 4];
		unsigned char *sl = &s[fromx * 4];
		for (int xi = 0; xi < w; xi++, dl += 4, sl += 4) {

			if (sl[3] == 0) continue;

			if (sl[3] == 255) {
				dl[0] = sl[0];
				dl[1] = sl[1];
				dl[2] = sl[2];
				continue;
			}

			int a = 255 - sl[3];


			dl[0] = (unsigned char)min((long(sl[0]) * long(sl[3]) + long(dl[0]) * a) / 255, 255);
			dl[1] = (unsigned char)min((long(sl[1]) * long(sl[3]) + long(dl[1]) * a) / 255, 255);
			dl[2] = (unsigned char)min((long(sl[2]) * long(sl[3]) + long(dl[2]) * a) / 255, 255);

		}
		s += ls;
		d += l;
	}

	_output->_dds->Unlock(NULL);

	return;
}

LuaEMethod(DDDrawMachine, drawText) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	CHECK_ARG(5, integer);
	CHECK_ARGCLASS(4, Font, _font);

	int x = (int)lua_tointeger(L, 1);
	int y = (int)lua_tointeger(L, 2);
	auto text = lua_tostring(L, 3);
	DWORD color = (DWORD)lua_tointeger(L, 5);

	__drawText(x, y, text, _font->_font, color);
	return 0;
}

LuaEMethod(DDDrawMachine, setOutput) {
	CHECK_ARGCLASS(1, DDOutput, output);
	_output = output;
	return 0;
}

LuaEMethod(DDDrawMachine, drawRect) {
	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	CHECK_ARG(3, integer);
	CHECK_ARG(4, integer);
	CHECK_ARG(5, integer);

	int x = (int)lua_tointeger(L, 1);
	int y = (int)lua_tointeger(L, 2);
	int w = (int)lua_tointeger(L, 3);
	int h = (int)lua_tointeger(L, 4);
	DWORD color = (DWORD)lua_tointeger(L, 5);

	__drawRect(x, y, w, h, color);

	return 0;
}

LuaEMethod(DDDrawMachine, width) {
	lua_pushinteger(L, _output->_width);
	return 1;
}

LuaEMethod(DDDrawMachine, height) {
	lua_pushinteger(L, _output->_height);
	return 1;
}

LuaMethods(DDDrawMachine) = {
	LuaMethodDesc(DDDrawMachine, drawText),
	LuaMethodDesc(DDDrawMachine, setOutput),
	LuaMethodDesc(DDDrawMachine, drawRect),
	LuaMethodDesc(DDDrawMachine, width),
	LuaMethodDesc(DDDrawMachine, height),
	{ 0,0 }
};

LuaMetaMethods(DDDrawMachine) = {
	{ 0,0 }
};

void module_dd_draw_machine(lua_State *L) {
	LuaClass<DDDrawMachine>::Register(L);
}