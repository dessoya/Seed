
local Child = require("Thread\\Child")
local PumpChild = Child:extend()
local MessagePump = require("MessagePump")

function PumpChild:setMessageNames(names)
	self.pump = MessagePump:new(names)
	self.pump:registerReciever(self)
end

function PumpChild:messageLoop(interval)

	self.work = true
	while self.work do

		if self.beforeReadMessage ~= nil then
			self:beforeReadMessage()
		end

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				self.pump:onMessage(self:get())
			end
		end

		if interval > 0 then
			sleep(interval)
		else
			threadYield()
		end

	end

end

function PumpChild:poolMessageLoop(interval)

	self.work = true
	while self.work do

		if self.beforeReadMessage ~= nil then
			self:beforeReadMessage()
		end

		local read = true
		while read do
			read = false
			if not self:empty() then
				read = true
				self.pump:onMessage(self:get())
			end
		end

		if self.poolQueue ~= nil then		
			read = true
			while read do
				read = false
				if not self:poolEmpty() then
					read = true
					local m = self:poolGet()
					if m ~= nil then
						self.pump:onMessage(m:get(0, -1, true))
					end
				end
			end 
		end

		if interval > 0 then
			sleep(interval)
		else
			threadYield()
		end

	end

end

return PumpChild