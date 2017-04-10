

local Control = require("Control")

local Wrap = Control.makeWrap(RenderObject)
Wrap:addEvents(Control.elibs.innerPosition)

local InnerRImage = Control.makeWrap(RImage)
InnerRImage:addEvents(Control.elibs.ohoverHand)
InnerRImage:addEvents(Control.elibs.innerPosition)

local InnerRText = Control.makeWrap(RText)
InnerRText:addEvents(Control.elibs.innerPosition)

local RadioBox = Control:extend()

function RadioBox:initialize(x, y, text, state, onChange_cb)

	self.name = text

	self.wrap = self:add(Wrap:new())
	self.wrap:set(#ROF_X, x, y)
	self.wrap:set(#ROF_BX, x, y)
	self.wrap:set(#ROF_SKIPHOVER, true)

	self.state = state
	self.onChange_cb = onChange_cb
	self.image = self:add(InnerRImage:new(0, 0, _get(#IMAGES):get("controls.png"), 0, 0, 0, 0))
	self.image:set(#ROF_LCLICK, function()
		self:onClick()
	end)
	self:updateImage()
	
	o = self:add(InnerRText:new(34, 8, text, _get(0), 0xffffff))
	o:set(#ROF_SKIPHOVER, true)

end

function RadioBox:setState(state)
	self.state = state
	self:updateImage()
end

function RadioBox:onClick()

	if self.state then return end

	self.state = true

	self:updateImage()
	if self.onChange_cb ~= nil then
		self.onChange_cb(self.name)
	end

end

function RadioBox:updateImage()
	if self.state then
		self.image:set(#RIF_IX, 38, 126, 45, 45)
		self.image:set(#ROF_W, 45, 45)
		self.image:set(#ROF_BX, -7, -6)
	else
		self.image:set(#RIF_IX, 0, 132, 30, 30)
		self.image:set(#ROF_W, 30, 30)
		self.image:set(#ROF_BX, 0, 0)
	end

	self.image:event(#ROF_ONPARENTCHANGEPOS, self.wrap)

end

return RadioBox