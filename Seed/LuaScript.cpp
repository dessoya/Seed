#include "windows.h"
// #include "string.h"
#include "LuaScript.h"
#include "LuaTypes.h"
#include "LuaCheck.h"
#include "LuaBaseClass.h"

#include "luajit\lua.hpp"
#include <map>
#include <list>

lua_State *_currentState;

// #define _DEBUG

#define MAX_SEARCH_DIRECTORY 16

#ifdef _DEBUG
#define dumpstack(name) { char b[1024]; sprintf(b, "[%s] stack size %d\n", name, lua_gettop(L)); OutputDebugStringA(b);	}
#endif

LuaFuncDesc *coreFuncs = new LuaFuncDesc[1024];
int coreFuncsCount = 0;


void LuaScript::addCoreFunction(const char *name, lua_CFunction func) {
	coreFuncs[coreFuncsCount] = { name, func };
	coreFuncsCount++;
}


LoadFile LuaScript::loadFileCallback = NULL;

typedef std::list<std::string> DirectoryList;
DirectoryList directoryList;

void LuaScript::addSearchDirectory(const char *directory) {
	directoryList.push_back(directory);
}

void LuaScript::setupLoadFileCallback(LoadFile loadFile) {
	loadFileCallback = loadFile;
}


const char *LuaScript::searchFile(lua_State *L, const char *filename, size_t *_sz) {
	
	if (loadFileCallback == NULL) {

		lua_pushfstring(L, "can't load file '%s.lua', loadFileCallback don't setuped", filename);
		return NULL;
	}

	const char *data = NULL;
	size_t sz;

	if (directoryList.size()) {
		for(DirectoryList::iterator it = directoryList.begin(), ite = directoryList.end(); it != ite; it++) {
			data = loadFileCallback((*it).c_str(), filename, &sz);
			if (data) {
				break;
			}
		}
	}
	else {
		data = loadFileCallback(NULL, filename, &sz);
	}

	if (data) {
		*_sz = sz;
		return data;
	}

	lua_pushfstring(L, "file '%s.lua' not found", filename);
	return NULL;
}

int errorHandler(lua_State* L) {

	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2); // remove debug

	lua_pcall(L, 0, 1, 0);

	lua_pushfstring(L, "%s\n%s", lua_tostring(L, 1), lua_tostring(L, -1));
	lua_remove(L, -2); // remove original error

	return 1;
}

/*
params: 1 string:filename

return: 2
bool:state true - loaded, false - not loaded

true:
result of file returens

false:
string:error

// use load external load file

*/



static int luaC_loadFile(lua_State *L) {

	CHECK_ARG(1, string);
	auto cs = lua_gettop(L);
	CHECK_OPTARG(2, integer);

	auto filename = lua_tostring(L, 1);

	auto nresults = LUA_MULTRET;
	if (lua_gettop(L) == 2) {
		nresults = (int)lua_tointeger(L, 2);
	}

	size_t sz;
	const char *text = LuaScript::searchFile(L, filename, &sz);
	if (text == NULL) {
		goto errorExit;
	}

	char resultFileName[256];
	sprintf_s(resultFileName, "%s.lua", filename);
	if (luaL_loadbufferx(L, text, sz, resultFileName,"t")) {

		lua_pushfstring(L, "loadFile syntax error:\n%s", lua_tostring(L, -1));
		lua_remove(L, -2);
		goto errorExit;
	}

	lua_pushcfunction(L, errorHandler);
	lua_insert(L, -2);

	if (lua_pcall(L, 0, nresults, -2)) {
		goto errorExit;
	}

	lua_remove(L, -2); // remove errorHandler

	lua_pushboolean(L, true);
	goto exit;

errorExit:
	lua_pushboolean(L, false);

exit:

	if (text) {
		delete text;
	}

	lua_insert(L, -2);
	return 2;
}

static int luaC_searchFile(lua_State *L) {

	CHECK_ARG(1, string);

	auto filename = lua_tostring(L, 1);

	size_t sz;
	const char *text = LuaScript::searchFile(L, filename, &sz);
	if (text == NULL) {
		lua_pushboolean(L, false);
		lua_insert(L, -2);
		return 2;
	}

	lua_pushboolean(L, true);
	lua_pushlstring(L, text, sz);
	delete text;

	return 2;
}


typedef std::map<std::string, InstallModule> ModulesMap;
ModulesMap modulesMap;

void LuaScript::addModule(const char *name, InstallModule installModule) {
	modulesMap.insert(std::pair<std::string, InstallModule>(name, installModule));
}

typedef std::map<unsigned short int, std::string> ModulesClassMap;
ModulesClassMap modulesClassMap;

std::string LuaScript::_currentInstallModuleName;
void LuaScript::makeClassModuleLink(unsigned short int classId) {
	modulesClassMap.insert(std::pair<unsigned short int, std::string>(classId, _currentInstallModuleName));
}

// void LuaScript::installModuleForClass(const char *className) {
void LuaScript::installModuleForClass(unsigned short int classId) {

	/*
	char b[1024];
	sprintf_s(b, "check module for class '%s'\n", className);
	OutputDebugStringA(b);
	*/

	// find module for class
	ModulesClassMap::iterator it = modulesClassMap.find(classId);
	if (it == modulesClassMap.end()) {
		lua_pushfstring(L, "can't find module for class '%s'", __classNameMap[classId]);
		lua_error(L);
	}

	auto moduleName = it->second;

	// install module if need
	std::set<std::string>::iterator mit = installedModules.find(moduleName);
	if (mit == installedModules.end()) {
		
		// sprintf_s(b, "install module '%s'\n", moduleName.c_str());
		// OutputDebugStringA(b);

		installedModules.insert(moduleName);
		std::map<std::string, InstallModule>::iterator iit = modulesMap.find(moduleName);
		iit->second(L);		
	}
	else {
		// sprintf_s(b, "module '%s' allready installed\n", moduleName.c_str());
		// OutputDebugStringA(b);
	}
}


static int luaC_installModule(lua_State *L) {

	CHECK_ARG(1, string);
	std::string moduleName(lua_tostring(L, 1));

	lua_getglobal(L, "__CClass");
	auto script = (LuaScript *)lua_touserdata(L, -1);
	lua_pop(L, 1);

	std::set<std::string>::iterator mit = script->installedModules.find(moduleName);
	if (mit != script->installedModules.end()) {
		return 0;
	}

	std::map<std::string, InstallModule>::iterator it = modulesMap.find(moduleName);
	if (it != modulesMap.end()) {

		script->installedModules.insert(moduleName);
		LuaScript::_currentInstallModuleName = it->first;
		it->second(L);
	}
	else {
		#ifdef _DEBUG
		// [string "core.lua"]:110 :
		lua_pushfstring(L, R"([file "%s"]%s:%d: module '%s' not found)", __FILE__, __FUNCTION__, __LINE__, moduleName.c_str());

		#else
		lua_pushfstring(L, "module '%s' not found", moduleName.c_str());
		#endif	
		lua_error(L);
	}

	return 0;
}

static int luaC_luaClassInstanceCount(lua_State *L) {
	lua_pushinteger(L, LuaBaseClass::_instanceCount);
	return 1;
}

#ifdef _DEBUG	


static int luaC_dprint(lua_State *L) {
	CHECK_ARG(1, string);
	char b[1024];
	sprintf_s(b, "%s\n", lua_tostring(L, 1));
	OutputDebugStringA(b);
	return 0;
}

static int luaC_userDataPtr(lua_State *L) {
	// CHECK_ARG(1, string);

	auto ud = (UserData *)lua_touserdata(L, 1);
	lua_pushinteger(L, (long long)ud->data);

	return 1;
}

static int luaC_userDataClass(lua_State *L) {
	// CHECK_ARG(1, string);

	auto ud = (UserData *)lua_touserdata(L, 1);
	auto bc = (LuaBaseClass *)ud->data;
	lua_pushstring(L, bc->getClassName());

	return 1;
}



#endif


/*
static int luaC_tableLen(lua_State *L) {
	lua_pushinteger(L, luaL_getn(L, 1));
	return 1;
}
*/

static int luaC_where(lua_State *L) {
	CHECK_ARG(1, integer);

	luaL_where(L, (int)lua_tointeger(L, 1));

	return 1;
}


extern LuaBaseClass *commonArray;

static int luaC_commonData(lua_State *L) {

	// OutputDebugStringA("get commonArray\n");

	commonArray->share(1);
	auto ud = static_cast<UserData *>(lua_newuserdata(L, sizeof(UserData)));
	ud->data = commonArray;  // store pointer to object in userdata
	ud->type = UDT_Class;
	commonArray->registerInNewState(L);

	return 1;
}

static int luaC_word(lua_State *L) {
	CHECK_ARG(1, integer);
	CHECK_ARG(2, integer);
	auto i = lua_tointeger(L, 1);
	auto pos = lua_tointeger(L, 2);
	long long mask = 0xffff;
	if (pos) {
		i >>= 16 * pos;
	}
	lua_pushinteger(L, (short int)(i & mask));
	return 1;
}

static int luaC_stringSplit(lua_State *L) {

	CHECK_ARG(1, string);
	CHECK_ARG(2, string);

	/*
	local result = {}
	local start, last = 1, str:len() + 1

	while start < last do
		local s, e = str : find(sep, start)
		if s ~= nil then
			table.insert(result, str:sub(start, s - 1))
			start = e + 1
		else
			break
		end
	end

	if start < last then
		table.insert(result, str:sub(start))
	end

	return result
	*/

	size_t len, seplen;
	auto text = lua_tolstring(L, 1, &len);
	auto sep = lua_tolstring(L, 2, &seplen);

	// dirty split lines hack :)

	bool splitByLines = false;
	if (seplen == 2 && sep[0] == '\r' && sep[1] == '\n') {
		splitByLines = true;
	}

	if (splitByLines) {
		if (strstr(text, sep) == NULL) {
			seplen = 1;
			sep = "\n";
		}
	}


	lua_newtable(L);

	size_t size = 0, pos = 0, start = 0;
	size_t sz;
	const char *line;

	while (start < len) {
		auto from = &text[start];
		auto seppos = strstr(from, sep);
		if (seppos) {

			line = from;
			sz = seppos - from;

			start += sz + seplen;

			size++;
			pos++;
			luaL_setn(L, -1, size);
			lua_pushlstring(L, line, sz);
			lua_rawseti(L, -2, (int)pos);
		}
		else {
			break;
		}
	}

	if (start < len) {

		line = &text[start];
		sz = len - start;

		size++;
		pos++;
		luaL_setn(L, -1, size);
		lua_pushlstring(L, line, sz);
		lua_rawseti(L, -2, (int)pos);
	}

	return 1;
}

/*
extern unsigned short int __classIterator;
extern int *__classCounts;
*/

static int luaC_luaClassInstanceCountByName(lua_State *L) {
	// CHECK_ARG(1, string);
	int args = 1 + (__classIterator - 1) * 2;
	int i = 1;
	lua_pushinteger(L, __classIterator - 1);
	while (i < __classIterator) {
		lua_pushstring(L, __classNameMap[i]);
		lua_pushinteger(L, __classCounts[i]);
		i++;
	}

	return args;
}


void LuaScript::installLib(const char *name, lua_CFunction func) {
	lua_pushcfunction(L, func);
	lua_pushstring(L, name);
	lua_call(L, 1, 0);
}

void LuaScript::installData(const char *name, void *data) {
	lua_pushlightuserdata(L, data);
	lua_setglobal(L, name);
}

LuaFuncDesc _baseLibs[] = {

	{ "_G", luaopen_base },
	{ LUA_TABLIBNAME, luaopen_table },
	{ LUA_MATHLIBNAME, luaopen_math },
	{ LUA_STRLIBNAME, luaopen_string },
	{ LUA_DBLIBNAME, luaopen_debug },

	{ 0, 0 }
};

LuaFuncDesc _baseFuncs[] = {

#ifdef _DEBUG	
	{ "dprint", luaC_dprint },
	{ "userDataPtr", luaC_userDataPtr },
	{ "userDataClass", luaC_userDataClass },

#endif

	{ "luaClassInstanceCount", luaC_luaClassInstanceCount },
	{ "luaClassInstanceCountByName", luaC_luaClassInstanceCountByName },
	{ "searchFile", luaC_searchFile },
	{ "loadFile", luaC_loadFile },
	{ "installModule", luaC_installModule },	
	{ "where", luaC_where },

	{ "commonData", luaC_commonData },
	{ "word", luaC_word },
	
	

	{ 0, 0 }
};


LuaScript::LuaScript() {

	L = luaL_newstate();

	if (luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE) == 0) {
		// lprint("luaJIT_setmode error");
	}

	// installData("__error_string", new std::string);
	installData("__CClass", this);

	for (auto f = _baseLibs; f->name; f++) {
		installLib(f->name, f->func);
	}	

	// luaL_getmetatable(L, "string");
	lua_getglobal(L, "string");

	lua_pushliteral(L, "split");
	lua_pushcfunction(L, luaC_stringSplit);
	lua_settable(L, -3);
	lua_pop(L, 1);

	/*
	lua_newtable(L);
	auto methods = lua_gettop(L);

	lua_getglobal(L, "table");
	auto metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methods);
	lua_settable(L, metatable);


	lua_pushcfunction(L, luaC_tableLen);
	lua_setfield(L, methods, "len");

	lua_pop(L, 2);
	*/
	

	for (auto f = _baseFuncs; f->name; f++) {
		lua_register(L, f->name, f->func);
	}

	for (int i = 0; i < coreFuncsCount; i++) {
		lua_register(L, coreFuncs[i].name, coreFuncs[i].func);
	}

	if (!loadFile("core")) {
		state = false;
		return;
	}

	state = true;
}

LuaScript::~LuaScript() {
	lua_close(L);
}

bool LuaScript::loadRequire(const char *filename, bool deleteResult) {
	
	lua_pushcfunction(L, errorHandler);
	lua_getglobal(L, "require");
	if (!lua_isfunction(L, -1)) {
		lua_pushstring(L, "require function not found");
		lua_error(L);
	}
	lua_pushstring(L, filename);
	if (lua_pcall(L, 1, 1, -3)) {
		lua_remove(L, -2); // remove errorHandler
		return false;
	}
	lua_remove(L, -2); // remove errorHandler
	if (deleteResult) {
		lua_remove(L, -1); // remove result
	}
	return true;
}

long long LuaScript::getResultAsInteger(int i) {
	auto l = lua_tointeger(L, i);
	lua_remove(L, i);
	return l;
}


bool LuaScript::loadFile(const char *filename) {

	lua_pushcfunction(L, errorHandler);
	lua_pushcfunction(L, luaC_loadFile);
	lua_pushstring(L, filename);
	lua_pushinteger(L, 1); // option parametr, count of load file result

	if (lua_pcall(L, 2, 2, -4)) {
		OutputDebugStringA("LuaScript::loadFile lua_pcall error\n");
		OutputDebugStringA(lua_tostring(L, -1));
		OutputDebugStringA("\n");
	}

	lua_remove(L, -3); // remove errorHandler

	auto s = lua_toboolean(L, -2); // get 1st result
	lua_remove(L, -2); // remove boolean(status)
	if (s) { // remove result of execution
		lua_remove(L, -1);
	}
	return s;
}

bool LuaScript::executeFunction(const char *functionName, LuaArgPusherCallback luaArgPusherCallback, void *ctx) {

	lua_pushcfunction(L, errorHandler);

	lua_getglobal(L, functionName);

	if (lua_isfunction(L, -1) == 0) {
		lua_pop(L, 2); // remove errorHandler and lua_getglobal(L, functionName);
		lua_pushfstring(L, "can't find function '%s'", functionName);
		return false;
	}

	int params = 0, errorId = -2;

	if (luaArgPusherCallback) {
		auto cnt = luaArgPusherCallback(L, ctx);
		params += cnt;
		errorId -= cnt;
	}

	// OutputDebugStringA("before pcall\n");
	auto r = lua_pcall(L, params, LUA_MULTRET, errorId);
	// OutputDebugStringA("after pcall\n");

	if (r) {
		return false;
	}
	
	return true;
}

std::string LuaScript::lastError() {
	
	auto c = lua_tostring(L, -1);
	if (c) {
		std::string error(c);
		lua_pop(L, 1);
		return error;
	}
	return "lastError == NULL";
}

const char *udtypename(int t) {
	switch (t) {
	case UDT_Nil: return "nil";
	case UDT_Boolean: return "boolean";
	case UDT_Integer: return "integer";
	case UDT_Double: return "double";
	case UDT_String: return "string";
	case UDT_Class: return "class";
	}
	return "unknown";
}