#include "Image.h"
#include <png\png.h>
#include "Data.h"


Image::Image(unsigned short int classId) : LuaBaseClass(classId) {
	setmt();
}

class Reader {
public:
	Reader(Data *d) {
		p = (char *)d->_data;
		sz = d->_size;
	}
	char *p;
	size_t sz;
	void read(char *b, size_t c) {
		memcpy(b, p, c);
		p += c;
	}
};

#define PNG_BYTES_TO_CHECK 4

void Image::__init(lua_State *L) {

	_lock->lock();

	// check args for variants
	// 1. from Data

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto data = (Data *)ud->data;

	Reader reader(data);

	char header[PNG_BYTES_TO_CHECK];
	png_structp _ptr;
	png_infop _info_ptr;


	reader.read(header, PNG_BYTES_TO_CHECK);

	if (png_sig_cmp((png_const_bytep)header, (png_size_t)0, PNG_BYTES_TO_CHECK)) {
		// abort_("[read_png_file] File %s is not recognized as a PNG file", filepath);
		lua_pushfstring(L, "png error 1");
		lua_error(L);
	}

	_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!_ptr) {
		// abort_("[read_png_file] png_create_read_struct failed");
		lua_pushfstring(L, "png error 2");
		lua_error(L);
	}

	_info_ptr = png_create_info_struct(_ptr);
	if (!_info_ptr) {
		// abort_("[read_png_file] png_create_info_struct failed");
		lua_pushfstring(L, "png error 3");
		lua_error(L);
	}

	// void png_set_read_fn(png_structp png_ptr, png_voidp io_ptr, png_rw_ptr read_data_fn);
	// void ReadDataFromInputStream(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead);

	png_set_read_fn(_ptr, &reader, [](png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
		png_voidp io_ptr = png_get_io_ptr(png_ptr);
		auto reader = (Reader *)io_ptr;
		reader->read((char *)outBytes, byteCountToRead);
	});

	if (setjmp(png_jmpbuf(_ptr))) {
		// abort_("[read_png_file] Error during init_io");
		lua_pushfstring(L, "png error 4");
		lua_error(L);
	}

	// png_init_io(_ptr, fp);
	png_set_sig_bytes(_ptr, PNG_BYTES_TO_CHECK);

	auto _number_of_passes = png_set_interlace_handling(_ptr);

	// png_read_png(_ptr, _info_ptr, PNG_TRANSFORM_EXPAND, NULL);
	png_read_png(_ptr, _info_ptr, 0, NULL);

	_width = png_get_image_width(_ptr, _info_ptr);
	_height = png_get_image_height(_ptr, _info_ptr);
	auto _color_type = png_get_color_type(_ptr, _info_ptr);
	auto _bit_depth = png_get_bit_depth(_ptr, _info_ptr);

	/*
	char bf[1024];
	sprintf_s(bf, "w %d, h %d\n", _width, _height);
	OutputDebugStringA(bf);
	*/

	if (_color_type & PNG_COLOR_MASK_COLOR)
		png_set_bgr(_ptr);

	/*
	lprint_IMAGE(filepath);
	lprint_IMAGE("_bit_depth " + boost::lexical_cast<std::string>((int)_bit_depth));
	lprint_IMAGE("_color_type " + boost::lexical_cast<std::string>((int)_color_type));
	*/

	auto _row_pointers = png_get_rows(_ptr, _info_ptr);

	_data = new char[_width * _height * 4];
	unsigned char *p = (unsigned char *)_data;

	for (unsigned int y = 0; y < _height; y++) {
		png_byte* row = _row_pointers[y];
		for (unsigned int x = 0; x < _width; x++, p += 4) {

			if (_color_type == PNG_COLOR_TYPE_RGB) {
				png_byte* ptr = &(row[x * 3]);

				p[0] = ptr[2];
				p[1] = ptr[1];
				p[2] = ptr[0];
				p[3] = 255;
			}
			else if (_color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
				png_byte* ptr = &(row[x * 4]);

				p[0] = ptr[2];
				p[1] = ptr[1];
				p[2] = ptr[0];
				p[3] = ptr[3];
				// lprint(inttostr((int)ptr[3]));
			}

		}
	}

	png_destroy_read_struct(&_ptr, NULL, &_info_ptr);

	_lock->unlock();

}

Image::~Image() {
	if (_data) {
		delete _data;
	}
}

class st {

public:

	int p1, p2;
	int s1, s2, s;

	st(int m, int d) {


		if (m < d) {
			s1 = d / m;
			s2 = d % m;
			s = m;
			p1 = 0;
			p2 = d / 2;
		}
		else {
			p1 = 0;
			p2 = 0;
			s1 = 0;
			s2 = 1;
			s = m;
		}

		afterInc();
	}

	void afterInc() {
		while (p2 >= s) {
			p2 -= s;
			p1++;
		}
	}

	void step() {
		p1 += s1;
		p2 += s2;
		afterInc();
	}
};

LuaEMethod(Image, scale) {
	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	auto m = lua_tointeger(L, 1);
	auto d = lua_tointeger(L, 2);

	auto image = LuaClass<Image>::__create(L);
	image->__scale(this, (DWORD)m, (DWORD)d);
	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Class;
	ud->data = image;
	image->registerInNewState(L);

	return 1;
}

void Image::__scale(Image *source, DWORD m, DWORD d) {

	_width = source->_width * m / d;
	_height = source->_height * m / d;

	_data = new char[_width * _height * 4];

	st ys(m, d);
	unsigned int sx = 0, sy = 0;

	for (unsigned int y = 0; y < _height; y++) {

		st xs(m, d);

		for (unsigned int x = 0; x < _width; x++) {

			auto dst = &_data[(x + y * _width) * 4];
			auto src = &source->_data[(sx + xs.p1 + ((sy + ys.p1) * source->_width)) * 4];

			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = src[3];

			xs.step();
		}

		ys.step();

	}
}

LuaMethods(Image) = {
	LuaMethodDesc(Image, scale),
	{ 0,0 }
};

LuaMetaMethods(Image) = {
	{ 0,0 }
};

void module_image(lua_State *L) {
	LuaClass<Image>::Register(L);
}