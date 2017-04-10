
installModule("thread")

require("Array")
require("Queue")

local Object = require("Object")
local ThreadHost = Object:extend()

function ThreadHost:initialize(filename, ownerQueue, ...)
	self.childQueue = Queue:new()
	self.thread = Thread:new("Thread\\ChildWrapper", Array:new(filename, ownerQueue, self.childQueue, ...))
end

function ThreadHost:send(...)
	self.childQueue:send(Array:new(...))
end

return ThreadHost