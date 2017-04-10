#include "Queue.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>


Queue::Queue(unsigned short int classId) : LuaBaseClass(classId), _empty(true) {
	setmt();
}

Queue::~Queue() {
	// OutputDebugStringA("~Queue\n");
}

LuaEMethod(Queue, empty) {
	lua_pushboolean(L, _empty);
	return 1;
}

LuaEMethod(Queue, send) {

	// lua_gc(L, LUA_GCCOLLECT, 0);

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto bc = (LuaBaseClass *)ud->data;
	bc->share(1);

	// boost::mutex::scoped_lock lock(_mutex);
	_lock->lock();
	_queue.push(bc);
	_empty = false;
	_lock->unlock();

	return 0;
}

LuaEMethod(Queue, get) {

	LuaBaseClass *bc = NULL;
	bool exitFlag = false;
	if (lua_gettop(L) > 0) {
		exitFlag = lua_toboolean(L, 1);
	}

	while (true) {
		boost::thread::yield();

		if (!_empty) {
			_lock->lock_shared();

			if (!_empty) {

				_lock->unlock_shared();
				_lock->lock();
				if (_queue.size() > 0) {
					bc = _queue.front();
					_queue.pop();
					_empty = _queue.empty();
					_lock->unlock();
					break;
				}
				else {
					_lock->unlock();
					if (exitFlag) {
						lua_pushnil(L);
						return 1;
					}
				}
			}
			else {
				_lock->unlock_shared();
				if (exitFlag) {
					lua_pushnil(L);
					return 1;
				}
			}

		}
		else {
			if (exitFlag) {
				lua_pushnil(L);
				return 1;
			}
		}
		Sleep(1);
	}
	
	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Class;
	ud->data = bc;
	bc->registerInNewState(L);

	return 1;
}



LuaMethods(Queue) = {
	LuaMethodDesc(Queue, empty),
	LuaMethodDesc(Queue, send),
	LuaMethodDesc(Queue, get),
	{ 0,0 }
};

LuaMetaMethods(Queue) = {
	{ 0,0 }
};

void module_queue(lua_State *L) {
	LuaClass<Queue>::Register(L);
}