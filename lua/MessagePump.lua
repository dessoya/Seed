local Object = require("Object")

local MessagePump = Object:extend()

function MessagePump:initialize(messageNames)
	if messageNames ~= nil then
		self.messageNames = messageNames
	else
		self.messageNames = { }
	end

	self.messageNames.onMessage = "onMessage"
	self.messageMap = { }
end

function MessagePump:addNames(messageNames)
	for message, messageName in pairs(messageNames) do
		self.messageNames[message] = messageName
	end
end

function MessagePump:onMessage(message, ...)

	local flag = false
	-- lprint("message " .. message)

	if self.messageMap[message] ~= nil then

		local recievers = self.messageMap[message]
		local method = self.messageNames[message]
		-- lprint("method " .. method)
		for i, r in ipairs(recievers) do
			local f = r[method]
			if f == nil then
				dprint("absent function '" .. method .. "'")
			else
				if f(r, ...) then
					flag = true
				end
			end
		end

	end

	if self.messageMap.onMessage ~= nil then

		local recievers = self.messageMap.onMessage
		for i, r in ipairs(recievers) do
			-- lprint(".onWindowMessage")
			if r:onMessage(message, ...) then
				flag = true
			end
		end

	end

	return flag
end

function MessagePump:unregisterReciever(reciever)
	for message, messageName in pairs(self.messageNames) do
		if reciever[messageName] ~= nil and type(reciever[messageName]) == "function" then
			local recievers = self.messageMap[message]
			for index, r in pairs(recievers) do
				if r == reciever then
					-- dprint("find with message " .. messageName)
					table.remove(recievers, index)
					break
				end
			end
		end
	end
end

function MessagePump:registerReciever(reciever)
	for message, messageName in pairs(self.messageNames) do
		if reciever[messageName] ~= nil and type(reciever[messageName]) == "function" then
			self:registerMessage(message, reciever)
		end
	end
end

function MessagePump:registerMessage(message, reciever)
	if self.messageMap[message] == nil then self.messageMap[message] = { } end
	-- lprint("message " .. message)
	table.insert(self.messageMap[message], reciever)
end


return MessagePump 