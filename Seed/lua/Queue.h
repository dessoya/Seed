#pragma once

#include "..\LuaScript.h"
#include "..\LuaBaseClass.h"
#include "..\LuaClass.h"
#include "..\LuaCheck.h"
#include <queue>

LuaClass(Queue)

	Queue(unsigned short int classId);
	~Queue();

	LuaIMethod(get);
	LuaIMethod(send);
	LuaIMethod(empty);

private:
	std::queue<LuaBaseClass *> _queue;
	volatile bool _empty;

};
