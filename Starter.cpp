// #define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include "Seed\LuaScript.h"

#define addModule(name) extern void module_##name(lua_State *L); LuaScript::addModule(#name, module_##name)

void installModules() {
	// install lua modules
	addModule(data);
	addModule(image);
	addModule(array);
	addModule(map);
	addModule(queue);
	addModule(thread);
	addModule(windows);
	addModule(terrain_map);
	addModule(dd_dd);
	addModule(dd_output);
	addModule(dd_draw_machine);
	addModule(render_object);
	addModule(rrect);
	addModule(rtext);
	addModule(rbox);
	addModule(rimage);
	addModule(rmap);
}

LuaScript *script = NULL;

std::string _seed_exit(std::string error, int code) {
	
	/*
	OutputDebugStringA("_seed_exit\n");
	OutputDebugStringA(error.c_str());
	OutputDebugStringA("_seed_exit\n");
	*/

	if (script) {

		if (script->executeFunction("exit")) {

		}
		else {
			error = std::string("start error:\n") + error + "\nexit error:\n" + script->lastError();
		}

		switch (code) {
		case 1:
		case 2:
			return error;
			break;

		case 3:
		case 4: {
			char b[1024];
			sprintf_s(b, "exit from thread\n");
			OutputDebugStringA(b);
			OutputDebugStringA(error.c_str());
			OutputDebugStringA("\n");

			exit(code);
		}
		}

		char b[1024];
		sprintf_s(b, "exit with unknown exit code '%d'\n", code);
		OutputDebugStringA(b);
		OutputDebugStringA(error.c_str());
		OutputDebugStringA("\n");

		exit(code);
	}
	else {

		OutputDebugStringA("exit without lua script obnject existing\n");
		OutputDebugStringA(error.c_str());
		OutputDebugStringA("\n");

		exit(code);
	}

	return "";
}


namespace Seed {
	
void addLuaSearchDirectory(const char *directory) {
	LuaScript::addSearchDirectory(directory);
}

void setupLuaLoadFileCallback(LoadFile loadFile) {
	LuaScript::setupLoadFileCallback(loadFile);
}
		
std::string start(HINSTANCE hInstance, int nCmdShow) {

	timeBeginPeriod(1);

	installModules();

	script = new LuaScript();
	if (!script->state) {
		return script->lastError();
	}

	typedef struct {
		HINSTANCE hInstance;
		int nCmdShow;
	} StartArgs;
	StartArgs startArgs = { hInstance, nCmdShow };

	if (script->loadRequire("start")) {
		if (script->executeFunction("start", [](lua_State *L, void *ctx) -> int {
			auto a = (StartArgs *)ctx;
			lua_pushinteger(L, (long long)a->hInstance);
			lua_pushinteger(L, (long long)a->nCmdShow);
			return 2;
		}, &startArgs)) {

			// get error status
			return "";
		}
		else {

			return _seed_exit(script->lastError(), 2);
		}
	}
	else {
		return _seed_exit(script->lastError(), 1);
	}

	return "";
}

void addCoreFunction(const char *name, lua_CFunction func) {
	LuaScript::addCoreFunction(name, func);
}

}
