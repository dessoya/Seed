#include "RMap.h"
#include "..\DD\DrawMachine.h"
#include "..\Windows.h"
#include "..\TerrainMap.h"

#define MAP_W 4096
#define MAP_H 3000
#define MAX_CELLS 256

char *bline = NULL;
char *gline = NULL;

void _memset(char *b, DWORD c, int l) {
	DWORD *d = (DWORD *)b;
	while (l--) {
		*d = c;
		d++;
	}
}

RMap::RMap(unsigned short int classId) : RenderObject(classId) {

	_vwp = _vhp = 0;
	_mxp = _myp = 0;
	_curScale = 0;

	if (bline == NULL) {
		bline = new char[4 * 1024];
		_memset(bline, 0, 1024);
	}

	if (gline == NULL) {
		gline = new char[4 * 1024];
		_memset(gline, 0, 1024);
		// _memset(gline, 0x00303030, 1024);
	}

	_map = new CellID[MAP_W * MAP_H];
	memset(_map, 0, sizeof(CellID) * MAP_W * MAP_H);
	// _map[MAP_W * 2 + 2] = 1;

	_vw = 3; _ox = 100; _mx = MAP_MID;
	_vh = 3; _oy = 100; _my = MAP_MID;
}

void RMap::__init(lua_State *L) {

	RenderObject::__init(L);

	lua_pushnil(L);
	auto nilindex = lua_gettop(L);
	_helper_set(L, nilindex, nilindex, RMF_CURH - 1);
	lua_pop(L, 1);

	_helper_set(L, 1, 3, RMF_TERRAINMAP);

	auto p = getData();

	_scales = (int)p[RMF_SCALES].i;
	_cells = (int)p[RMF_CELLS].i;

	_scaleInfo = new ScaleInfo[_scales];
	_images = new PCellInfo[_scales];

	for (int i = 0; i < _scales; i++) {
		_images[i] = new CellInfo[_cells];
		memset(_images[i], 0, sizeof(CellInfo) * _cells);
	}

	p[ROF_X].type = UDT_Integer;
	p[ROF_X].i = 0;

	p[ROF_Y].type = UDT_Integer;
	p[ROF_Y].i = 0;

	p[ROF_BX].type = UDT_Integer;
	p[ROF_BX].i = p[ROF_X].i;

	p[ROF_BY].type = UDT_Integer;
	p[ROF_BY].i = p[ROF_Y].i;

	p[ROF_W].type = UDT_Integer;
	p[ROF_W].i = 0;

	p[ROF_H].type = UDT_Integer;
	p[ROF_H].i = 0;

	p[ROF_SKIPHOVER].b = true;

	p[RMF_CURSCALE].type = UDT_Integer;
	p[RMF_CURSCALE].i = 0;

	p[RMF_CURW].type = UDT_Integer;
	p[RMF_CURW].i = 0;

	p[RMF_CURH].type = UDT_Integer;
	p[RMF_CURH].i = 0;


	_loadFromWorldMap(L);
}

void RMap::_loadFromWorldMap(lua_State *L) {
	auto p = getData();
	auto _terrainMap = (TerrainMap *)p[RMF_TERRAINMAP].baseClass;
	for (lint y = 0; y < _vh; y++) {
		for (lint x = 0; x < _vw; x++) {			
			_map[x + y * MAP_W] = _terrainMap->__getCell(_mx + x, _my + y);
		}
	}
}

LuaEMethod(RMap, setCoords) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);

	auto x = lua_tointeger(L, 1);
	auto y = lua_tointeger(L, 2);

	_lock->lock();

	_mxp = x;
	_myp = y;

	long long _tmx = x / 1024;;
	long long _tmy = y / 1024;

	auto p = getData();
	_curScale = (int)p[RMF_CURSCALE].i;

	auto s = &_scaleInfo[_curScale];

	_ox = x % 1024;
	_oy = y % 1024;

	_ox = floor((double)_ox / s->k);
	_oy = floor((double)_oy / s->k);

	if (_tmx != _mx || _tmy != _my) {
		_mx = _tmx;
		_my = _tmy;
		_loadFromWorldMap(L);
	}

	_lock->unlock();
	return 0;
}


RMap::~RMap() {

}


void RMap::__drawSelf(DDDrawMachine *drawMachine) {

	// auto p = getData();

/*
	auto _x = p[ROF_X].i;
	auto _y = p[ROF_Y].i;
	auto _fromx = p[RIF_IX].i;
	auto _fromy = p[RIF_IY].i;
	auto _w = p[RIF_IW].i;
	auto _h = p[RIF_IH].i;
	auto _image = (Image *)p[RIF_IMAGE].baseClass;

	drawMachine->__drawImage((int)_x, (int)_y, _image, (int)_fromx, (int)_fromy, (int)_w, (int)_h);
*/
	// OutputDebugStringA("RMap::__drawSelf\n");
	drawMachine->__lock();

	// _lock->lock_shared();

	unsigned char *p = (unsigned char *)drawMachine->_surface;
	long l = drawMachine->_pitch;

	auto _p = getData();
	_curScale = (int)_p[RMF_CURSCALE].i;

	auto s = &_scaleInfo[_curScale];
	_cw = s->w;
	_ch = s->h;

	// long l2 = l / 4;
	// lprint(std::string("l ") + inttostr(l) + " l2 " + inttostr(l2));

	auto _dwidth = drawMachine->_output->_width;
	auto _dheight = drawMachine->_output->_height;

	for (int y = 0; y < _vh; y++) {

		int ty = y * _ch - _oy;

		if (ty >= _dheight) break;

		auto c = &_map[MAP_W * y];

		for (int x = 0; x < _vw; x++) {

			// lets draw cell
			auto cid = *c;
			auto flags = cid >> 12;
			cid &= 0xfff;

			/*
			0 = gline
			ABSENT_CELL = bline
			*/

			bool _add = true;

			int tx = x * _cw - _ox;
			if (tx >= _dwidth) break;


			DWORD *imageLine;
			long l3;

			if (cid == 0 || cid == 0xfff || cid >= _cells || _images[_curScale][cid].image == NULL) {
				_add = false;
				if (cid) {
					imageLine = (DWORD *)bline;
				}
				else {
					imageLine = (DWORD *)gline;
				}
			}
			else {
				auto i = &_images[_curScale][cid];
				auto image = i->image;
				l3 = image->_width;
				auto xo = 0, yo = 0;
				if (tx < 0) {
					xo = tx;
				}
				if (ty < 0) {
					yo = ty;
				}
				imageLine = (DWORD *)&image->_data[((i->y - yo) * image->_width + i->x - xo) * 4];
			}

			// fix tx and l1
			int l1 = _cw;
			if (tx < 0) {
				l1 += tx;
				tx = 0;
			}

			if (tx + l1 > _dwidth) {
				l1 = _cw - ((tx + l1) - _dwidth);
			}

			int l11 = _ch;
			if (ty < 0) {
				l11 += ty;
				/*
				if (_add) {
				imageLine -= l3 * ty;
				}
				*/
				// ty = 0;
			}

			if (ty + l11 > _dheight) {
				l11 = _ch - ((ty + l11) - _dheight);
			}

			auto sLine = &p[(ty < 0 ? 0 : ty) * l + tx * 4];

			if (cid == 0 || cid == 0xfff) {
				for (int y1 = 0; y1 < l11; y1++) {

					DWORD *imageLine1 = imageLine;
					DWORD *sLine1 = (DWORD *)sLine;

					memcpy(sLine, imageLine, l1 * 4);

					sLine += l;
				}

			}
			else {
				if (flags == 1) {
					for (int y1 = 0; y1 < l11; y1++) {

						DWORD *imageLine1 = imageLine;
						DWORD *sLine1 = (DWORD *)sLine;

						for (int x1 = 0; x1 < l1; x1++) {
							DWORD p = *imageLine1;
							p >>= 1;
							p &= 0x7f7f7f7f;
							*sLine1 = p;
							sLine1++;
							imageLine1++;
						}


						imageLine += l3;
						sLine += l;
					}
				}
				else {
					for (int y1 = 0; y1 < l11; y1++) {

						memcpy(sLine, imageLine, l1 * 4);

						imageLine += l3;
						sLine += l;
					}
				}

			}
			c++;
		}
	}

	drawMachine->__unlock();

}

#define CELL_SIZE 1024
#define CELL_BITS 10

LuaEMethod(RMap, setScaleInfo) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	CHECK_ARG(3, integer);

	auto s = lua_tointeger(L, 1);
	auto w = lua_tointeger(L, 2);
	auto h = lua_tointeger(L, 3);

	_scaleInfo[s].w = (int)w;
	_scaleInfo[s].h = (int)h;
	_scaleInfo[s].k = (double)CELL_SIZE / (double)w;

	return 0;
}

LuaEMethod(RMap, setCellImage) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	CHECK_ARGCLASS(3, Image, _image);
	auto scale = lua_tointeger(L, 1);
	auto id = lua_tointeger(L, 2);
	_images[scale][id].image = _image;
	_images[scale][id].x = 0;
	_images[scale][id].y = 0;

	return 0;
}

/*
LuaEMethod(RMap, setScale) {
	CHECK_ARG(1, integer);
	_lock->lock();
	_curScale = (int)lua_tointeger(L, 1);
	_lock->unlock();
	return 0;
}
*/

LuaEMethod(RMap, setViewSize) {

	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);

	int w = (int)lua_tointeger(L, 1);
	int h = (int)lua_tointeger(L, 2);

	_lock->lock();

	_vwp = w;
	_vhp = h;

	auto p = getData();
	_curScale = (int)p[RMF_CURSCALE].i;

	p[RMF_CURW].i = w;
	p[RMF_CURH].i = h;

	auto __vw = w / _scaleInfo[_curScale].w + 2;
	auto __vh = h / _scaleInfo[_curScale].h + 2;

	bool needLoad = false;
	if (__vw > _vw || __vh > _vh) {
		needLoad = true;
	}
	_vw = __vw;
	_vh = __vh;

	// lprint("w " + inttostr(_vw) + " h " + inttostr(_vh));

	if (needLoad) {
		_loadFromWorldMap(L);
	}

	_lock->unlock();

	return 0;
}



LuaAMethods(RMap, [](lua_State *L, int methods) { LuaAddMethods(RenderObject); }) = {
	LuaMethodDesc(RMap, setScaleInfo),
	LuaMethodDesc(RMap, setCellImage),
	LuaMethodDesc(RMap, setViewSize),
	LuaMethodDesc(RMap, setCoords),
	// LuaMethodDesc(RMap, setScale),
	{ 0,0 }
};

LuaMetaMethods(RMap) = {
	// LuaMethodDesc(Array, __tostring),
	// LuaMethodDesc(Font, __newindex),
	{ 0,0 }
};


void module_rmap(lua_State *L) {
	LuaClass<RMap>::Register(L);
}
