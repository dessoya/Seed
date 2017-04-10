#pragma once

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

// shared_mutex for more read then write

// mutex for more write then read

typedef boost::shared_mutex LuaLock;

/*
typedef boost::unique_lock< LuaLock >  LuaWriteLock;
typedef boost::shared_lock< LuaLock >  LuaReadLock;
*/
