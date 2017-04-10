
require("KeyCodes")
require("Windows")

local Object = require("Object")
local Keys = Object:extend()

function Keys:initialize()

	self.recievers = { }
	self.keyStates = { }
	for i = 1, 255 do
		self.keyStates[i] = false
	end

end

function Keys:registerReciever(r)
	table.insert(self.recievers, r)
end

function Keys:attachToPump(pump)

	pump:addNames({
		[#WM_SYSKEYDOWN]	= "onSysKeyDown",
		[#WM_SYSKEYUP]		= "onSysKeyUp",
		[#WM_KEYDOWN]		= "onKeyDown",
		[#WM_KEYUP]			= "onKeyUp",
		[#WM_ACTIVATE]		= "onWindowActive"
	})

	pump:registerReciever(self)	
end

function Keys:onSysKeyDown(key)
	self:keyDown(key)
end

function Keys:onSysKeyUp(key)
	self:keyUp(key)
end

function Keys:onKeyDown(key)
	self:keyDown(key)
end

function Keys:onKeyUp(key)
	self:keyUp(key)
end

function Keys:onWindowActive()
	self.keyStates[#Key_Alt] = false
end

function Keys:keyDown(key)
	if not self.keyStates[key] then
		self.keyStates[key] = true
		self:keyPressed(key, self.keyStates[#Key_Alt])
	end
end

function Keys:keyUp(key)
	if self.keyStates[key] then
		self.keyStates[key] = false
		self:keyUnPressed(key, self.keyStates[#Key_Alt])
	end
end

function Keys:keyPressed(key, alt)
	for index, r in pairs(self.recievers) do
		r:keyPressed(key, alt)
	end
end

function Keys:keyUnPressed(key, alt)
	for index, r in pairs(self.recievers) do
		r:keyUnPressed(key, alt)
	end
end

return Keys