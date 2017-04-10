#pragma once

#include "lua.h"
#include "LuaLock.h"
#include <boost\atomic.hpp>

#include <windows.h>

extern char **__classNameMap;
extern unsigned short int __classIterator;
extern int *__classCounts;


class LuaBaseClass {

public:

#ifdef _DEBUG
#endif

	volatile static long long _instanceCount;

	LuaLock *_lock;
	unsigned short int _classId;
	// mutable
	boost::atomic<short int> _share;
	// , _share;

	// const char *_class;

#ifdef _DEBUG_ARRAY
	/*
	static long long idIterator;
	long long _id;
	*/
#endif

	static int _Wsetmt(lua_State *L);
	static unsigned short int getClassId(const char *name);
	static int getClassCountByName(const char *name);

	inline void registerInNewState(lua_State *L) {
		// luaL_getmetatable(L, _class);  // lookup metatable in Lua registry
		luaL_getmetatable(L, __classNameMap[_classId]);		
		lua_setmetatable(L, -2);
	}

	inline short int share(short int delta, bool withlock = true) {

		
		if (delta > 0) {
			_share.fetch_add(1, boost::memory_order_relaxed);
			return 0;
		}
		

		auto r = _share.fetch_sub(1, boost::memory_order_release);
		if (r == 1) {
			boost::atomic_thread_fence(boost::memory_order_acquire);
			return 0;
		}
		 
		return r - 1;
	}

	inline void setmt() {
		if (_lock == NULL) {
			_lock = new LuaLock;
		}
	}

	inline const char *getClassName() {
		return __classNameMap[_classId];
	}

	LuaBaseClass(unsigned short int classId);
	virtual void __init(lua_State *L) { };
	virtual ~LuaBaseClass();
};

typedef LuaBaseClass *PLuaBaseClass;