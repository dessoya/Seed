
installModule("thread")

require("Array")
require("Queue")

local Object = require("Object")
local ThreadPool = Object:extend()

function ThreadPool:initialize(filename, ownerQueue, size, ...)
	
	self.pool = { }
	self.poolQueue = Queue:new()
	self.size = size
	for i = 1, size do
		local info = { }
		info.childQueue = Queue:new()
		Thread:new("Thread\\PoolChildWrapper", Array:new(filename, i, ownerQueue, self.poolQueue, info.childQueue, ...))
		self.pool[i] = info
	end
end

function ThreadPool:send(...)
	self.poolQueue:send(Array:new(...))	
end

function ThreadPool:sendAll(...)
	for i = 1, self.size do
		local info = self.pool[i]
		info.childQueue:send(Array:new(...))
	end
end

--[[
function ThreadPool:done()
	self.poolQueue = nil
	for i = 1, self.size do
		local info = self.pool[i]
		info.childQueue = nil
		info.id = nil
		self.pool[i] = nil
	end
	self.pool = nil
end
]]


return ThreadPool