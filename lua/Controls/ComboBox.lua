
local RendererHost = require("DD\\Renderer\\Host")
local Control = require("Control")

local Wrap = Control.makeWrap(RenderObject)
Wrap:addEvents(Control.elibs.innerPosition)
Wrap:addEvents(Control.elibs.pushEvents)

local InnerRBox = Control.makeWrap(RBox)
InnerRBox:addEvents(Control.elibs.innerPosition)
InnerRBox:addEvents(Control.elibs.ohoverHand)
InnerRBox:addEvents(Control.elibs.pushEvents)

local InnerRBox2 = Control.makeWrap(RBox)
InnerRBox2:addEvents(Control.elibs.innerPosition)

local InnerRText = Control.makeWrap(RText)
InnerRText:addEvents(Control.elibs.innerPosition)
InnerRText:addEvents(Control.elibs.pushEvents)

local InnerRBox3 = Control.makeWrap(RBox)
InnerRBox3:addEvents(Control.elibs.innerPosition)
InnerRBox3:addEvents(Control.elibs.pushEvents)
InnerRBox3:addEvents({
	onHover = function(self)
		RendererHost.setCursor(#Cursor_Hand)
		self:set(#RBF_COLOR,0xbcbcbc)
	end,
	onHoverLost = function(self)
		RendererHost.setCursor(#Cursor_Arrow)
		self:set(#RBF_COLOR,0x7c7c7c)
	end
})

local Item = Control:extend()

function Item:initialize(x, y, w, h, caption, onClick, current)
	local c2 = 0x7c7c7c
	if current then c2 = 0xffffff end
	local wrap = self:add(InnerRBox3:new(x, y, w, h, 0x7c7c7c, 1, c2))
	wrap:set(#ROF_LCLICK, function()
		onClick(caption)
	end)
	self:add(InnerRText:new(3, 3, caption, _get(0), 0xffffff))
end

local ComboBox = Control:extend()

function ComboBox:initialize(x, y, w, h, onChange_cb)

	self.onChange_cb = onChange_cb
	self.h = h
	self.w = w
	self.list = { }

	self.wrap = self:add(Wrap:new())
	self.wrap:set(#ROF_X, x, y)
	self.wrap:set(#ROF_BX, x, y)
	self.wrap:set(#ROF_SKIPHOVER, true)

	local captionBoxControl = self:add(InnerRBox:new(0, 0, w, h, 0x7c7c7c, 1, 0xbcbcbc))

	captionBoxControl:set(#ROF_LCLICK, bind(self.onClick, self))

	self.captionControl = captionBoxControl:add(InnerRText:new(5, 5, "", _get(0), 0xffffff))
	captionBoxControl:add(InnerRText:new(w - 20, 8, "^", _get(0), 0xffffff))

	self.opened = false

end


function ComboBox:hideList()
	if self.opened then
		self:del(self.listControl)
		self.opened = false
	end	
end

function ComboBox:setList(list, index)
	self.list = list
	self:setListIndex(index)
end

function ComboBox:setListIndex(index)
	local item = self.list[index]
	self.currentItem = item
	self.currentIndex = index
	self.captionControl:set(#RTF_TEXT, item.caption)
	if self.onChange_cb ~= nil then
		self.onChange_cb(item)
	end	
end

function ComboBox:onClick()
	if self.opened then
		self.opened = false
		self:del(self.listControl)
		self.listControl = nil
	else
		self.opened = true
		self.listControl = self:add(InnerRBox2:new(0, self.h, self.w, 11 + 21 * table.getn(self.list), 0x7c7c7c, 1, 0xbcbcbc))		
		for i, item in ipairs(self.list) do
			local current = i == self.currentIndex
			local index = i
			self.listControl:add(Item:new(5, 5 + (i - 1) * 21, self.w - 10, 20, item.caption, function(caption)
				self:onItemClick(index, caption)
			end, current))
		end
	end
end

function ComboBox:onItemClick(index, caption)
	-- dprint("ComboBox:onItemClick " .. index )
	local item = self.list[index]
	self.currentItem = item
	self.currentIndex = index
	self:del(self.listControl)
	self.opened = false
	self.captionControl:set(#RTF_TEXT, caption)
	if self.onChange_cb ~= nil then
		self.onChange_cb(item)
	end
end

return ComboBox