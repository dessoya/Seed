#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

#pragma comment(lib, "Seed.lib")
#pragma comment(lib, "Winmm.lib")

#define FL_PACK 1
#define FL_ENCODED 2

#include <map>

#include "..\Seed\Pack.h"

extern "C" {
# include "luajit\lua.h"
# include "luajit\lauxlib.h"
# include "luajit\lualib.h"
}

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>


#define CHECK_integer(n) (lua_isnumber(L, n) == 0)
#define CHECK_string(n) (lua_isstring(L, n) == 0)

#define CHECK_ARG(n, t) if( CHECK_##t(n) ) { luaL_typerror(L, n, #t); }

namespace Seed {
	std::string start(HINSTANCE hInstance, int nCmdShow);

	void addCoreFunction(const char *name, lua_CFunction func);

	void addLuaSearchDirectory(const char *directory);

	typedef const char *(*LoadFile)(const char *dir, const char *filename, size_t *sz);
	void setupLuaLoadFileCallback(LoadFile loadFile);

	void _encode(void *b, int l, int *pos); 
	
	class FilePack;
	class FilePackEntry {
		// std::string filename;
		long fpos;
		char flags;
		int index;
		DWORD datasize, unpacksize;
	public:
		friend FilePack;
		size_t getSize() {
			return unpacksize;
		}
		FilePackEntry(long _fpos, int _index, char _flags, DWORD _datasize, DWORD _unpacksize) : fpos(_fpos), index(_index), flags(_flags), datasize(_datasize), unpacksize(_unpacksize) {
		}

	};

	typedef std::map<std::string, FilePackEntry *> FileMap;

#define FILESCOUNT 16
	class FilePack {
		FILE *f[FILESCOUNT];
		FileMap fileMap;
		boost::shared_mutex _lock[FILESCOUNT];
		boost::shared_mutex _locki;
		int index;
	public:

		void *load(FilePackEntry *e) {

			_locki.lock();
			auto i = index;
			index++;
			if (index >= FILESCOUNT) {
				index = 0;
			}
			_locki.unlock();

			void *b = malloc(e->datasize);

			_lock[i].lock();
			fseek(f[i], e->fpos, SEEK_SET);

			fread(b, e->datasize, 1, f[i]);
			_lock[i].unlock();

			if (e->flags & FL_ENCODED) {
				int pos = e->index;
				_encode(b, e->datasize, &pos);
			}

			if (e->flags & FL_PACK) {
				void *d = malloc(e->unpacksize + 1);
				Unpacker unpacker((char *)d, e->unpacksize, (char *)b, e->datasize);
				delete b;
				b = d;

				/*
				char bb[1024];
				sprintf_s(bb, "%d %d\n", (int)e->unpacksize, (int)e->datasize);
				OutputDebugStringA(bb);
				((char *)b)[e->unpacksize] = 0;
				OutputDebugStringA((char *)b);
				OutputDebugStringA("\n");
				*/

			}

			return b;
		}

		FilePackEntry *get(const char *filename) {
			auto iter = fileMap.find(filename);
			if (iter == fileMap.end()) {
				return 0;
			}
			return iter->second;
		}

		FilePack(char *filename) {
			
			index = 0;
			for (int i = 0; i < FILESCOUNT; i++) {
				fopen_s(&f[i], filename, "rb");
			}

			// scan
			int index_pos = 0;
			int index = 0;

			char b[1024];
			while (true) {
				char fl;
				fread(&fl, 1, 1, f[0]);
				_encode(&fl, 1, &index_pos);
				if (fl == 0) {
					break;
				}

				fread(b, fl, 1, f[0]);
				_encode(&b, fl, &index_pos);
				b[fl] = 0;
				std::string filename(b);

				char flags;
				fread(&flags, 1, 1, f[0]);
				_encode(&flags, 1, &index_pos);

				DWORD datasize;
				fread(&datasize, 4, 1, f[0]);
				_encode(&datasize, 4, &index_pos);

				DWORD unpacksize = datasize;
				if (flags & FL_PACK) {					
					fread(&unpacksize, 4, 1, f[0]);
					_encode(&unpacksize, 4, &index_pos);

				}

				auto fpos = ftell(f[0]);
				fseek(f[0], fpos + datasize, SEEK_SET);

				// sprintf_s(b, "file %s\n", filename.c_str());
				// OutputDebugStringA(b);

				fileMap.insert(std::pair<std::string, FilePackEntry *>(filename, new FilePackEntry(fpos, index, flags, datasize, unpacksize)));

				index++;
			}

		}
	};

	
}
