#pragma once

#include "LuaTypes.h"

typedef void(*AncestorMethodFiller)(lua_State *L, int methods);

template <typename T> class LuaClass {
public:

	static void Register(lua_State *L) {

		// if (T::__classId == 0) {
		T::__classId = LuaBaseClass::getClassId(T::className);
		//}

		// LuaScript::makeClassModuleLink(__classNameMap[T::__classId]);
		LuaScript::makeClassModuleLink(T::__classId);

		lua_newtable(L);
		int methods = lua_gettop(L);

		luaL_newmetatable(L, T::className);
		int metatable = lua_gettop(L);

		// store method table in globals so that
		// scripts can add functions written in Lua.

		lua_pushstring(L, T::className);
		lua_pushvalue(L, methods);
		lua_settable(L, LUA_GLOBALSINDEX);

		/*
		lua_pushliteral(L, "__metatable");
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);  // hide metatable from Lua getmetatable()
		*/

		lua_pushliteral(L, "__index");
		lua_pushvalue(L, methods);
		lua_settable(L, metatable);

		lua_pushliteral(L, "__gc");
		lua_pushcfunction(L, gc_T);
		lua_settable(L, metatable);


		for (LuaFuncDesc *l = T::metamethods; l->name; l++) {
			lua_pushstring(L, l->name);
			lua_pushcfunction(L, l->func);
			lua_settable(L, metatable);
		}

		lua_pushliteral(L, "new");
		lua_pushcfunction(L, new_T);
		lua_settable(L, methods);       // add new_T to method table

		lua_pushliteral(L, "setmt");
		lua_pushcfunction(L, LuaBaseClass::_Wsetmt);
		lua_settable(L, methods);       // add new_T to method table
										
										// call ancestor methods filler
		if (T::ancestorMethodFiller) {
			T::ancestorMethodFiller(L, methods);
		}

		// fill method table with methods from class T
		for (LuaFuncDesc *l = T::methods; l->name; l++) {
			lua_pushstring(L, l->name);
			lua_pushcfunction(L, l->func);
			lua_settable(L, methods);
		}

		lua_pop(L, 2);  // drop metatable and method table
	}

	static T *__create(lua_State *L) {		
		T *obj = new T(T::__classId);
		// obj->_class = T::className;
		// obj->_classId = T::__classId;
		return obj;
	}

private:
	LuaClass();

	static unsigned short int __classId;

	// create a new T object and
	// push onto the Lua stack a userdata containing a pointer to T object
	static int new_T(lua_State *L) {
		lua_remove(L, 1);   // use classname:new(), instead of classname.new()
		T *obj = new T(T::__classId);  // call constructor for T objects
		// obj->_class = T::className;
		// obj->_classId = T::__classId;
		obj->__init(L);

		UserData *ud = static_cast<UserData *>(lua_newuserdata(L, sizeof(UserData)));
		ud->data = obj;  // store pointer to object in userdata
		ud->type = UDT_Class;
		luaL_getmetatable(L, T::className);  // lookup metatable in Lua registry
		lua_setmetatable(L, -2);
		return 1;  // userdata containing pointer to T object
	}

	// garbage collection metamethod
	static int gc_T(lua_State *L) {

		UserData *ud = static_cast<UserData *>(lua_touserdata(L, 1));
		T *obj = (T *)ud->data;

		// OutputDebugStringA("share(-1)\n");
		auto share = obj->share(-1);
		

		/*
		char b[1024];
		sprintf_s(b,"gc_T %s id %d _share %d\n", obj->_class, (int)obj->_id, obj->_share);
		OutputDebugStringA(b);
		*/

		if (share == 0) {
			_currentState = L;
			delete obj;  // call destructor for T objects
		}
		return 0;
	}

};

#define LuaChildClass(c,p) class c : public p { public: static AncestorMethodFiller ancestorMethodFiller; static unsigned short int __classId; static const char className[]; static LuaFuncDesc methods[], metamethods[]; 
#define LuaChild2Class(c,p1,p2) class c : public p1, public p2 { public: static AncestorMethodFiller ancestorMethodFiller; static unsigned short int __classId; static const char className[]; static LuaFuncDesc methods[], metamethods[]; 

#define LuaClass(c) LuaChildClass(c, LuaBaseClass)

#define LuaMethod(c, a) static int _W##a(lua_State *L) { auto ud = (UserData *)lua_touserdata(L, 1); auto p = (c *)ud->data; lua_remove(L, 1); return p->a(L); } inline int a(lua_State *L)
#define LuaIMethod(a) static int _W##a(lua_State *L); int a(lua_State *L);
#define LuaEMethod(c, a) int c::_W##a(lua_State *L) { auto ud = (UserData *)lua_touserdata(L, 1); auto p = (c *)ud->data; lua_remove(L, 1); return p->a(L); } int c::a(lua_State *L)

#define LuaMethods(c) AncestorMethodFiller c::ancestorMethodFiller = NULL; unsigned short int c::__classId = 0; const char c::className[] = #c; LuaFuncDesc c::methods[]
#define LuaAMethods(c, f) AncestorMethodFiller c::ancestorMethodFiller = f; unsigned short int c::__classId = 0; const char c::className[] = #c; LuaFuncDesc c::methods[]
#define LuaMethodDesc(c, name) {#name, c::_W##name}
#define LuaMethodDesc_(c, name) { #name, c::_W_##name}

// #define LuaAncestorFiller(c, f) AncestorMethodFiller c::ancestorMethodFiller = f;
#define LuaAddMethod(c,m) lua_pushstring(L, #m); lua_pushcfunction(L, c::_W##m); lua_settable(L, methods);

#define LuaAddMethods(c) \
/* L->installModuleForClass(__classId); */\
lua_getglobal(L, "__CClass");\
auto script = (LuaScript *)lua_touserdata(L, -1);\
lua_pop(L, 1);\
script->installModuleForClass(c::__classId);\
lua_getglobal(L, #c); \
/* OutputDebugStringA("LuaAddMethods " #c "\n");\
OutputDebugStringA(lua_typename(L, lua_type(L, -1)));\
OutputDebugStringA("\n");  */ \
auto t = lua_gettop(L); \
lua_pushnil(L); \
while (lua_next(L, t) != 0) { \
\
	if(lua_isfunction(L, -1) || lua_iscfunction(L, -1)) { \
		auto s = lua_tostring(L, -2);\
		/* OutputDebugStringA("func ");\
		OutputDebugStringA(s);\
		OutputDebugStringA("\n"); */ \
		if(strcmp(s, "new") != 0) { \
\
			lua_pushvalue(L, -2); \
			lua_insert(L, -2); \
		\
			lua_settable(L, methods); \
			continue;\
		}\
	}\
	lua_pop(L, 1); \
\
} \
lua_pop(L, 2);

// for (LuaFuncDesc *l = c::methods; l->name; l++) { lua_pushstring(L, l->name); lua_pushcfunction(L, l->func); lua_settable(L, methods); }


#define LuaMetaMethods(c) LuaFuncDesc c::metamethods[]


#define ARGCLASS(n, c, v) auto ud##n = (UserData *)lua_touserdata(L, n); auto v = (c *)ud##n->data;