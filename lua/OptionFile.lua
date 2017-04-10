installModule("map")
installModule("data")
installModule("queue")
local Object = require("Object")
local OptionFile = Object:extend()

function OptionFile:initialize(filename)
	self.filename = filename
	self.groups = { }
	self.queue = Queue:new()
	local status, d = pcall(loadFileData, filename)
	if status then
		local s = d:tostring()	

		local lines = s:split("\r\n")
		local group
		for i, line in ipairs(lines) do
	   		for p in line:gmatch("[[](%w+)[]]") do
	   			group = self:getGroup(p)
	      		break
	   		end

	   		for k,v in line:gmatch("%s*(%w+)%s*[=]%s*(%w+)%s*") do
	      		if v == "true" then
	      			v = true
	      		elseif v == "false" then
	      			v = false
	      		else
	      			local n = tonumber(v)
	      			if n ~= nil then
	      				v = n
	      			end
	      		end
	      		group:set(k, v)
	   			break
	   		end

		end
	end
end

function OptionFile:getGroup(groupName)
	local group = self.groups[groupName]
	if group == nil then
		self.queue:send(Array:new())
		group = Map:new()		
		group:addQueue(self.queue)
		self.groups[groupName] = group
	end
	return group
end

function OptionFile:update()
	local read = false
	while not self.queue:empty() do
		local m = self.queue:get()
		read = true
	end
	if read then
		local text = ""		
		for groupName, group in pairs(self.groups) do
			text = text .. "[" .. groupName .. "]\n"
			group:foreach(function(key, value)
				local v = ""
				if type(value) == "boolean" then
					if value then
						v = v .. "true"
					else
						v = v .. "false"
					end
				else
					v = v .. value
				end
				text = text .. key .. " = " .. v .. "\n"
			end)
		end
		local d = Data:new(text)
		d:saveToFile(self.filename)
	end
end

return OptionFile