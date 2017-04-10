#include <string>
#include <list>
#include "Pack.h"
#include "zlib\zlib.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

char *tchar2char(wchar_t *t) {
	auto l = wcslen(t) + 1;
	char *s = new char[l];
	// wcstombs_s(s, t, wcslen(t) + 1);
	size_t sz;
	wcstombs_s(&sz, s, l, t, l);
	return s;
}

wchar_t *char2tchar(char *s) {
	size_t size = strlen(s) + 1;
	wchar_t *wbuf = new wchar_t[size];
	size_t outSize;
	mbstowcs_s(&outSize, wbuf, size, s, size - 1);
	return wbuf;
}

namespace Seed {

	unsigned char *psig = (unsigned char *)"CoOoLLL pROt3Cti0n !!11";
	auto psig_len = strlen((char *)psig);

	void _encode(void *b, int l, int *pos) {
		auto *bb = (unsigned char *)b;

		if ((*pos) >= psig_len) {
			*pos = (*pos) % psig_len;
		}

		while (l--) {
			*bb ^= psig[*pos];
			(*pos)++;
			if ((*pos) >= psig_len) {
				(*pos) = 0;
			}
			bb++;
		}
	}

	void fwrite(void *b, size_t l, FILE *f, int *enc_pos, bool copy = true) {

		void *c = NULL;

		if (copy) {
			void *c = malloc(l);
			memcpy(c, b, l);
			b = c;
		}

		_encode(b, (int)l, enc_pos);
		::fwrite(b, l, 1, f);

		if (c) {
			free(c);
		}
	}


	typedef std::list<std::string> FileList;

	FileList getFileList(char *dirname) {
		FileList fileList;

		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		char b[1024];
		sprintf_s(b, "%s\\*", dirname);

		hFind = FindFirstFile(char2tchar(b), &ffd);

		if (INVALID_HANDLE_VALUE == hFind) {
			sprintf_s(b, "Error: FindFirstFile(%s)\n", dirname);
			OutputDebugStringA(b);
			return fileList;
		}

		do {

			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

				char *filename = tchar2char(ffd.cFileName);
				if (filename[0] != '.') {
					sprintf_s(b, "%s\\%s", dirname, filename);
					auto list = getFileList(b);
					for (auto it = list.begin(), eit = list.end(); it != eit; it++) {
						fileList.push_back(*it);
					}
				}

			}
			else {
				// check for .lua
				char *filename = tchar2char(ffd.cFileName);
				bool accept = true;

				/*
				if (wc) {
					accept = false;
					if (strlen(filename) > strlen(wc) && strcmp(wc, &filename[strlen(filename) - strlen(wc)]) == 0) {
						accept = true;
					}
				}
				*/

				if (accept) {
					sprintf_s(b, "%s\\%s", dirname, filename);
					fileList.push_back(b);
					/*
					list->size = ffd.nFileSizeLow;
					list->filename = makecopy(b);
					list->next = NULL;
					*/
				}

			}
		} while (FindNextFile(hFind, &ffd) != 0);

		return fileList;
	}

}

Packer::Packer(char *data, long l, int level) {

	_data = NULL;
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	if (deflateInit(&strm, level) != Z_OK) {
		// eprint("error: deflateInit");
		return;
	}

	strm.avail_in = l;
	strm.next_in = (Bytef *)data;

	_data = new char[l + 1024];
	strm.avail_out = l + 1024;
	strm.next_out = (Bytef *)_data;

	auto r = deflate(&strm, Z_FINISH);
	if (!(r == Z_OK || r == Z_STREAM_END)) {
		// eprint("error: deflate");
	}

	_l = (l + 1024) - strm.avail_out;
	deflateEnd(&strm);

}

Unpacker::Unpacker(char *dest, long destSize, char *src, long srcSize) {

	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	auto ret = inflateInit(&strm);

	strm.avail_in = srcSize;
	strm.next_in = (Bytef *)src;

	strm.avail_out = destSize;
	strm.next_out = (Bytef *)dest;

	ret = inflate(&strm, Z_NO_FLUSH);

	switch (ret) {
	case Z_NEED_DICT:
		ret = Z_DATA_ERROR;
	case Z_DATA_ERROR:
	case Z_MEM_ERROR:
		(void)inflateEnd(&strm);
		// eprint("error: inflate");
		return;
	}

	(void)inflateEnd(&strm);
}
