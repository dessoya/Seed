#pragma once

#pragma comment(lib, "zlibstat.lib")
#ifndef _DEBUG
#pragma comment(lib, "libzipmasm.lib ")
#endif

#pragma comment(lib, "Seed.lib")

#include <string>
#include <list>

#define FL_PACK 1
#define FL_ENCODED 2

class Packer {
public:
	char *_data;
	long _l;

	Packer(char *data, long l, int level = 9);
	~Packer() { if (_data) delete _data; }

};

class Unpacker {
public:
	Unpacker(char *dest, long destSize, char *src, long srcSize);

};

namespace Seed {

	typedef std::list<std::string> FileList;

	FileList getFileList(char *dirname);
	void _encode(void *b, int l, int *pos);
	void fwrite(void *b, size_t l, FILE *f, int *enc_pos, bool copy = true);


}