
local Control = require("Control")

local InnerRBox = Control.makeWrap(RBox)
InnerRBox:addEvents(Control.elibs.innerPosition)
InnerRBox:addEvents(Control.elibs.hoverHand)
InnerRBox:addEvents(Control.elibs.pushEvents)



local InnerRText = Control.makeWrap(RText)
InnerRText:addEvents(Control.elibs.innerPosition)
InnerRText:addEvents(Control.elibs.pushEvents)

local Button = Control:extend()

function Button:initialize(x, y, w, h, text, onClick)

	local o = self:add(InnerRBox:new(x, y, w, h, 0x7c7c7c, 1, 0x7c7c7c))
	if onClick ~= nil then
		o:set(#ROF_LCLICK, function()
			onClick()
		end)
	end
	
	o = self:add(InnerRText:new(5, 5, text, _get(0), 0xffffff))
	o:set(#ROF_SKIPHOVER, true)

end

return Button