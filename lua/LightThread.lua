
local Object = require("Object")

local LightThread = Object:extend()

local LightThreadIterator = 1
local LightThreadStack = { }

function LightThread:initialize(func)
	self.created = true
	self.id = LightThreadIterator
	LightThreadIterator = LightThreadIterator + 1
	self.co = coroutine.create(func)
	self:resume()
end

function LightThread:resume(...)

	if self.created ~= nil then
		self.created = nil
		table.insert(LightThreadStack, self)
	end


	LightThread.current = self
	local status, result, r1, r2, r3, r4 = coroutine.resume(self.co, ...)
	if not status then
		error(result)
	end

	if coroutine.status(self.co) == "dead" then
		table.remove(LightThreadStack, table.getn(LightThreadStack))
		if table.getn(LightThreadStack) > 0 then
			local lt = LightThreadStack[table.getn(LightThreadStack)]
			return lt:resume(result, r1, r2, r3, r4)
		end
	end
	return result, r1, r2, r3, r4

end

LightThread.yield = function (...)
	return coroutine.yield(...)
end

return LightThread