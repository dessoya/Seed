#include "..\LuaScript.h"
#include "..\LuaTypes.h"
#include "..\LuaBaseClass.h"
#include "..\LuaClass.h"
#include "..\LuaCheck.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <boost/thread.hpp>

typedef struct {

	char *filename;
	void *data;

} ThreadParams;

std::string _seed_exit(std::string error, int code);

DWORD WINAPI LuaThreadFunction(LPVOID lpParam) {

	// _set_se_translator(MiniDumpFunction);
	try  // this try block allows the SE translator to work
	{
		auto p = (ThreadParams *)lpParam;

		// auto a = p->a;

		auto script = new LuaScript();

		if (!script->state) {
			auto s = _seed_exit(script->lastError(), 3);
			delete p;
			delete script;

			return 0;
		}

		if (script->loadRequire(p->filename)) {
			if (script->executeFunction("thread", [](lua_State *L, void *ctx) -> int {
				auto a = (ThreadParams *)ctx;

				auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
				ud->type = UDT_Class;
				ud->data = a->data;
				auto bc = (LuaBaseClass *)a->data;
				bc->registerInNewState(L);

				return 1;
			}, p)) {

			}
			else {
				std::string err = std::string("executeFunction thread error:\n" + script->lastError());
				/*
				OutputDebugStringA("executeFunction thread error:\n");
				OutputDebugStringA(script->lastError().c_str());
				OutputDebugStringA("\n");
				*/
				auto s = _seed_exit(err, 4);
			}
		}
		else {
			std::string err = std::string("loadRequire ") + p->filename + " error: " + script->lastError();

			/*
			OutputDebugStringA("loadRequire ");
			OutputDebugStringA(p->filename);
			OutputDebugStringA("error:");		
			OutputDebugStringA(script->lastError().c_str());
			OutputDebugStringA("\n");
			*/

			auto s = _seed_exit(err, 3);
		}


		delete p;
		delete script;

		return 0;
	}
	catch (...)
	{
		// lua_error(L, "catch ...");
		return -1;
	}
}

LuaClass(Thread)

	Thread(unsigned short int classId) : LuaBaseClass(classId) { }

	virtual void __init(lua_State *L) {

		CHECK_ARG(1, string);
		CHECK_ARG(2, array);

		auto ud = (UserData *)lua_touserdata(L, 2);
		auto bc = (LuaBaseClass *)ud->data;
		bc->share(1);

		auto p = new ThreadParams{ (char *)lua_tostring(L, 1), ud->data };

		_ht = CreateThread(NULL, 0, LuaThreadFunction, p, 0, NULL);

	}

	~Thread() {
		// OutputDebugStringA("~Thread\n");
	}

private:
	HANDLE _ht;
};

LuaMethods(Thread) = {
	// LuaMethodDesc_(Queue, register),
	{ 0,0 }
};

LuaMetaMethods(Thread) = {
	// LuaMethodDesc(Queue, __tostring),
	// LuaMethodDesc(Queue, __newindex),
	{ 0,0 }
};

/*
static int luaC_threadSetName(lua_State *L) {

	CHECK_ARG(1, string);

	// Logger::setThreadName(lua_tostring(L, 1));

	return 0;
}
*/

static int luaC_threadYield(lua_State *L) {
	boost::thread::yield();
	return 0;
}

static int luaC_sleep(lua_State *L) {
	CHECK_ARG(1, integer);
	Sleep((DWORD)lua_tointeger(L, 1));
	return 0;
}

static int luaC_queryPerformanceFrequency(lua_State *L) {
	LARGE_INTEGER l;
	QueryPerformanceFrequency(&l);
	lua_pushinteger(L, l.QuadPart);
	return 1;
}

static int luaC_queryPerformanceCounter(lua_State *L) {
	LARGE_INTEGER l;
	QueryPerformanceCounter(&l);
	lua_pushinteger(L, l.QuadPart);
	return 1;
}


void module_thread(lua_State *L) {
	LuaClass<Thread>::Register(L);

	lua_register(L, "threadYield", luaC_threadYield);
	lua_register(L, "sleep", luaC_sleep);

	lua_register(L, "queryPerformanceFrequency", luaC_queryPerformanceFrequency);
	lua_register(L, "queryPerformanceCounter", luaC_queryPerformanceCounter);

	
	// lua_register(L, "threadSetName", luaC_threadSetName);
	
}