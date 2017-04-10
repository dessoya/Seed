
local Object = require("Object")
local Child = Object:extend()

function Child:initialize(index, ownerQueue, poolQueue, childQueue, ...)

	self.index = index
	self.poolQueue = poolQueue
	self.ownerQueue = ownerQueue
	self.childQueue = childQueue

	if self.init ~= nil then
		self:init(...)
	else
		self.args = { ... }
	end

end

function Child:send(...)
	self.ownerQueue:send(Array:new(...))
end

function Child:empty()
	return self.childQueue:empty()
end

function Child:get()
	return self.childQueue:get():get(0, -1, true)
end

function Child:poolEmpty()
	return self.poolQueue:empty()
end

function Child:poolGet()
	return self.poolQueue:get(true)
end

return Child