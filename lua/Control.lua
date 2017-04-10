
installModule("render_object")
installModule("rrect")
installModule("rbox")
installModule("rtext")
installModule("rimage")
installModule("rmap")

#const ROF_ID				0
#const ROF_PARENT 			1
#const ROF_CHILDS			2
#const ROF_DRAWORDER		3
#const ROF_X				4
#const ROF_Y				5
#const ROF_W				6
#const ROF_H				7
#const ROF_BX				8
#const ROF_BY				9
#const ROF_SKIPHOVER		10
#const ROF_ISHOVER			11
#const ROF_LPUSHSTATE		12

#const ROEVENTS				13


#const ROF_ONMOUSEMOVE		(#ROEVENTS + 0)
#const ROF_ONHOVER			(#ROEVENTS + 1)
#const ROF_ONHOVERLOST		(#ROEVENTS + 2)
#const ROF_ONVIEWSIZE		(#ROEVENTS + 3)
#const ROF_ONADD			(#ROEVENTS + 4)
#const ROF_ONPARENTCHANGEPOS (#ROEVENTS + 5)
#const ROF_LDOWN			(#ROEVENTS + 6)
#const ROF_LUP				(#ROEVENTS + 7)
#const ROF_LCLICK			(#ROEVENTS + 8)

#const ROPROPS				(#ROEVENTS + 9)

#const RRF_COLOR			(#ROPROPS + 0)

#const RTF_TEXT				(#ROPROPS + 0)
#const RTF_FONT				(#ROPROPS + 1)
#const RTF_COLOR			(#ROPROPS + 2)

#const RBF_COLOR			(#ROPROPS + 0)
#const RBF_SIZE				(#ROPROPS + 1)
#const RBF_RCOLOR			(#ROPROPS + 2)

#const RIF_IMAGE			(#ROPROPS + 0)
#const RIF_IX				(#ROPROPS + 1)
#const RIF_IY				(#ROPROPS + 2)
#const RIF_IW				(#ROPROPS + 3)
#const RIF_IH				(#ROPROPS + 4)

#const RMF_TERRAINMAP		(#ROPROPS + 0)
#const RMF_SCALES			(#ROPROPS + 1)
#const RMF_CELLS			(#ROPROPS + 2)
#const RMF_CURSCALE			(#ROPROPS + 3)
#const RMF_CURW				(#ROPROPS + 4)
#const RMF_CURH				(#ROPROPS + 5)



local RendererHost = require("DD\\Renderer\\Host")

local Object = require("Object")
local Control = Object:extend()

function Control:add(o)

	if self.firstChild == nil then
		self.firstChild = o
		return o
	end

	self.firstChild:add(o)

	return o
end

function Control:del(o)
	self.firstChild:del(o)
end

local eventMap = {
	onHover 			= #ROF_ONHOVER,
	onHoverLost 		= #ROF_ONHOVERLOST,
	onViewSize 			= #ROF_ONVIEWSIZE,
	onAdd 				= #ROF_ONADD,
	onParentChangePos 	= #ROF_ONPARENTCHANGEPOS,
	onLDown				= #ROF_LDOWN,
	onLUp				= #ROF_LUP,
	onLClick			= #ROF_LCLICK

}

function __addEvents(self, events)
	for key, val in pairs(events) do
		if type(val) == "function" then
			self.__events[key] = val
		end
	end
end

function __new(self, ...)

	local obj

	if self.__events.initArgs ~= nil then
		obj = self.__userdata:new(self.__events.initArgs(...))
	else
		obj = self.__userdata:new(...)
	end

	for key, val in pairs(self.__events) do
		local id = eventMap[key]
		if id ~= nil then
			obj:set(id, val)
		end
	end

	return obj
end

function Control.makeWrap(ro)

	local o = {
		__userdata = ro,
		__events = { },		
		new = __new,
		addEvents = __addEvents
	}

	return o
end

Control.elibs = { }

-- centring ability

function calcPos(o, w, h)
	local ow, oh = o:get(#ROF_W, 2)
	local x, y = (w - ow) / 2, (h - oh) / 4
	o:set(#ROF_X, math.floor(x), math.floor(y))	
	o:cevent(#ROF_ONPARENTCHANGEPOS, o)
end

Control.elibs.centring = {
	onAdd = function(self)
		-- dprint("onAdd")
		calcPos(self, RendererHost.getViewSize())
	end,
	onViewSize = function(self, w, h)
		-- dprint("onViewSize")
		calcPos(self, w, h)
	end
}

-- InnerPosition

local InnerPosition = { }
function InnerPosition:onParentChangePos(parent)	
	local x, y = parent:get(#ROF_X, 2)
	local bx, by = self:get(#ROF_BX, 2)
	-- dprint("InnerPosition:onParentChangePos " .. self:get(#ROF_ID) .. ", " .. x .. "x" .. y .. ", " .. bx .. "x" .. by)
	self:set(#ROF_X, x + bx, y + by)

	self:cevent(#ROF_ONPARENTCHANGEPOS, self)
end

function InnerPosition:onAdd(parent)
	local x, y = parent:get(#ROF_X, 2)
	local bx, by = self:get(#ROF_BX, 2)
	self:set(#ROF_X, x + bx, y + by)

	self:cevent(#ROF_ONPARENTCHANGEPOS, self)
end

Control.elibs.innerPosition = InnerPosition



local HoverHand = { }
function HoverHand:onHover()
	RendererHost.setCursor(#Cursor_Hand)
	self:set(#RBF_RCOLOR,0xbcbcbc)
end

function HoverHand:onHoverLost()
	RendererHost.setCursor(#Cursor_Arrow)
	self:set(#RBF_RCOLOR,0x7c7c7c)
end
Control.elibs.hoverHand = HoverHand

local OHoverHand = { }
function OHoverHand:onHover()
	RendererHost.setCursor(#Cursor_Hand)
end

function OHoverHand:onHoverLost()
	RendererHost.setCursor(#Cursor_Arrow)
end
Control.elibs.ohoverHand = OHoverHand

local PushEvents = { }
function PushEvents:onLDown()
	local x, y = self:get(#ROF_X, 2)
	self:set(#ROF_X, x + 1, y + 1)
	self:cevent(#ROF_LDOWN)
end

function PushEvents:onLUp(l)
	local x, y = self:get(#ROF_X, 2)
	self:set(#ROF_X, x - 1, y - 1)
	self:cevent(#ROF_LUP)
end

Control.elibs.pushEvents = PushEvents

return Control