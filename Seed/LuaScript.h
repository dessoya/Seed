#pragma once

#include "lua.h"

#pragma comment(lib, "lua51.lib")

#include <string>
#include <set>

typedef struct { const char *name; lua_CFunction func; } LuaFuncDesc;
typedef struct { int type; void *data; } UserData;

typedef int (*LuaArgPusherCallback)(lua_State *L, void *ctx);
typedef void (*InstallModule)(lua_State *L);
typedef const char *(*LoadFile)(const char *dir, const char *filename, size_t *sz);

int errorHandler(lua_State* L);

class LuaScript {

	lua_State* L;
	void installLib(const char *name, lua_CFunction func);
	void installData(const char *name, void *data);

	static LoadFile loadFileCallback;

public:

	static void addCoreFunction(const char *name, lua_CFunction func);

	std::set<std::string> installedModules;
	bool state;

	LuaScript();
	virtual ~LuaScript();

	lua_State *getState() { return L;  }

	static void addSearchDirectory(const char *directory);
	static void setupLoadFileCallback(LoadFile loadFile);
	static const char *searchFile(lua_State *L, const char *filename, size_t *_sz);
	static void addModule(const char *name, InstallModule installModule);
	static void makeClassModuleLink(unsigned short int classId);
	static std::string _currentInstallModuleName;

	// void installModuleForClass(const char *className);
	void installModuleForClass(unsigned short int classId);

	bool loadFile(const char *filename);
	bool loadRequire(const char *filename, bool deleteResult = true);
	bool executeFunction(const char *functionName, LuaArgPusherCallback luaArgPusherCallback = NULL, void *ctx = NULL);

	long long getResultAsInteger(int i);

	std::string lastError();
};

extern lua_State *_currentState;
const char *udtypename(int t);