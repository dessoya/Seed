#pragma once

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