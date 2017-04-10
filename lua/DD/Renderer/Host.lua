
require("Windows")

installModule("dd_dd")
installModule("dd_output")

require("DD\\Renderer\\Messages")

#const OT_Windowed			1
#const OT_Fullscreen		2

#const DDSCL_NORMAL			0x008
#const DDSCL_EXCLUSIVE		0x010
#const DDSCL_FULLSCREEN		0x001
#const DDSCL_MULTITHREADED	0x400

#const LS_NONE				0
#const LS_BOTTOM			1

#const SE_AFTERADD			0
#const SE_AFTERDEL			1


local ThreadHost = require("Thread\\Host")

local Object = require("Object")
local RendererHost = Object:extend()

local LightThread = require("LightThread")

local singleton

function RendererHost:initialize(dd, wnd, pump, queue, ro, mainQueue, renderInfoCollectorFile, optionFile)

	singleton = self
	self.ro = ro
	self.optionFile = optionFile

	self.isMouseLeaved = true
	self.mainQueue = mainQueue

	self.lstate = 0
	self.mx = 0
	self.my = 0

	self.ignoreMovePosition = true

	-- self.f = queryPerformanceFrequency()
	-- self.moveTime = queryPerformanceCounter()	

	self.switchModeInProgress = false

	self.returnIterator = 1
	self.dd = dd
	local w, h = self.getScreenSize()
	self.w = w
	self.h = h
	self.currentW = w
	self.currentH = h
	self.wnd = wnd
	self.returnMap = { }
	
	local g = self.optionFile:getGroup("video")
	if g:get("windowX") == -1 then		
		local x, y, w, h = self.wnd:getPlacement()
		g:set("windowX", x)
		g:set("windowY", y)
		g:set("windowWidth", w)
		g:set("windowHeight", h)
	end

	pump:addNames({
		[#MR_Answer]		= "onAnswer",
		[#MR_WndMinimized]	= "onWndMinimized",
		[#MR_WndRestored]	= "onWndRestored",

		[#WM_SIZE]			= "onSize",
		[#WM_ACTIVATE]		= "onWindowActive",

		[#WM_MOUSEMOVE]		= "onMouseMove",
		[#MR_MouseButtons]	= "onMouseButtons",
		[#WM_MOUSELEAVE]	= "onMouseLeave",
		[#WM_MOVE]			= "onWindowMove"

	})
	pump:registerReciever(self)

	self.renderingThread = ThreadHost:new("DD\\Renderer\\Rendering", queue, ro, renderInfoCollectorFile)

	-- self.renderer = DDRenderer:new(dd, wnd)

	self.mode = 0
	self.window_output = DDOutput:new(#OT_Windowed, wnd)
	self.fullscreen_output = DDOutput:new(#OT_Fullscreen)
	-- self.wnd:savePlacement()
end

function RendererHost:onWindowMove(x, y)

	if (x == 0 and y == 0) or x == 33536 then return end
	-- dprint("RendererHost:onWindowMove " .. x .. "x" .. y)

	if not self.ignoreMovePosition then
		local g = self.optionFile:getGroup("video")
		local x, y = self.wnd:getPlacement();
		g:set("windowX", x)
		g:set("windowY", y)
	end

end

function RendererHost:onAnswer(returnId, ...)
	local lt = self.returnMap[returnId]
	-- dprint("RendererHost:onAnswer " .. returnId)
	--dprint("onAnswer lt.id " .. lt.id)
	if lt ~= nil then
		lt:resume(...)
	end
end

function RendererHost:getMessageId()
	local returnId = self.returnIterator
	self.returnIterator = self.returnIterator + 1
	return returnId
end

function RendererHost:sendWithReturn(messageId, ...)
	--dprint("RendererHost:sendWithReturn")
	local returnId = self:getMessageId()
	local lt = LightThread.current
	--dprint("sendWithReturn lt.id " .. lt.id)
	self.returnMap[returnId] = lt
	self.renderingThread:send(messageId, returnId, ...)
	--dprint("3")
end

function RendererHost:stopRendering()
	--dprint("1")
	self:sendWithReturn(#MR_StopRendering)
	--dprint("2")
	return LightThread.yield()
end

function RendererHost:startRendering()
	self:sendWithReturn(#MR_StartRendering)
	return LightThread.yield()
end

function RendererHost:sendNewOutput(output, mode)
	self:sendWithReturn(#MR_Output, output, mode)
	return LightThread.yield()
end


function RendererHost:enableWindowedMode()

	LightThread:new(function()

		-- dprint("RendererHost:enableWindowedMode")
		self.switchModeInProgress = true
		-- LightThread.protect("RendererHost:switchMode")

		-- stop render thread
		self:stopRendering()

		self.window_output:release()
		self.fullscreen_output:release()

		self.dd:setCooperativeLevel(self.wnd, #DDSCL_NORMAL + #DDSCL_MULTITHREADED)

		local w, h = self.getScreenSize()
		if w ~= self.w or h ~= self.h then
			self.dd:setMode(self.w, self.h)
		end

		self.wnd:addStyle(#WS_OVERLAPPEDWINDOW)
		-- self.wnd:restorePlacement()

		local g = self.optionFile:getGroup("video")
		self.wnd:setPlacement(g:get("windowX"), g:get("windowY"), g:get("windowWidth"), g:get("windowHeight"))

		self.window_output:create(self.dd)

		-- send to render new output
		self:sendNewOutput(self.window_output, "windowed")
		self.mode = #OT_Windowed

		-- start render thread
		self:startRendering()

		self.switchModeInProgress = false
		-- dprint("RendererHost:enableWindowedMode done")

		self:sendOnSizeToObjects(self.window_output:getSize())

		self.ignoreMovePosition = false

	end)

end

function RendererHost:enableFullscreenMode(w, h)

	LightThread:new(function()

		self.ignoreMovePosition = true

		--dprint("RendererHost:enableFullscreenMode")
		self.switchModeInProgress = true

		-- LightThread.protect("RendererHost:switchMode")

		-- stop render thread
		-- self.wnd:savePlacement()

		self:stopRendering()

		self.window_output:release()
		self.fullscreen_output:release()

		self.wnd:removeStyle(#WS_OVERLAPPEDWINDOW)
		self.wnd:show(#SW_NORMAL)
		-- self.wnd:maximizePos()

		self.dd:setCooperativeLevel(self.wnd, #DDSCL_EXCLUSIVE + #DDSCL_FULLSCREEN + #DDSCL_MULTITHREADED)

		if w ~= nil then
			if w ~= self.currentW or h ~= self.currentH then
				self.currentW = w
				self.currentH = h
				self.dd:setMode(w, h)			
			end
		else
			self.currentW = self.w
			self.currentH = self.h
		end

		self.fullscreen_output:create(self.dd)

		-- send to render new output
		self:sendNewOutput(self.fullscreen_output, "fullscreen")
		self.mode = #OT_Fullscreen

		-- start render thread
		self:startRendering()
		self.switchModeInProgress = false

		self:sendOnSizeToObjects(self.fullscreen_output:getSize())

		self.ignoreMovePosition = false

	end)

end

function RendererHost:quit()

	LightThread:new(function()

		self:sendWithReturn(#M_Quit)
		local name = LightThread.yield()
		--dprint("thread " .. name .. " done")
	end)

end

function RendererHost:switchFullscreen()

	return LightThread:new(function()

		--dprint("RendererHost:switchFullscreen")

		-- LightThread.protect("RendererHost:alttab")

		if self.mode == #OT_Windowed then

			LightThread.yield(self:enableFullscreenMode())
			-- self:enableFullscreenMode()
			--dprint("asd 2")
			return true, self.currentW, self.currentH

		elseif self.mode == #OT_Fullscreen then

			LightThread.yield(self:enableWindowedMode())
			-- self:enableWindowedMode()
			--dprint("asd 1")
			return false, self.w, self.h

		end

	end)

end

function RendererHost:onWndMinimized()
	if self.mode ~= #OT_Windowed then return end	
	if not self.switchModeInProgress then
		--dprint("RendererHost:onWndMinimized")
		self.renderingThread:send(#MR_StopRendering, -1)
	end
end

function RendererHost:onWndRestored()
	if self.mode ~= #OT_Windowed then return end	
	if not self.switchModeInProgress then
		--dprint("RendererHost:onWndRestored")
		self.renderingThread:send(#MR_StartRendering, -1)
	end
end

function RendererHost:onSize(w, h)

	if self.mode ~= #OT_Windowed then return end	
	if self.switchModeInProgress then return end

	self.newSize = true
	self:startSizing()
	
end

function RendererHost:sendOnSizeToObjects(w, h)
	self.ro:event(#ROF_ONVIEWSIZE, w, h)
end

function RendererHost:onWindowActive(activate)

	-- dprint("RendererHost:onActivate " .. activate)

	if self.mode ~= #OT_Fullscreen then return end	

	if activate == 0 then
		self.ignoreMovePosition = true
		if self.switchModeInProgress then return end
		LightThread:new(function()

			self.switchModeInProgress = true
			--dprint("onActivate hide")

			self:stopRendering()
			--dprint("onActivate hide 1")
			self.window_output:release()
			--dprint("onActivate hide 2")
			self.fullscreen_output:release()
			--dprint("onActivate hide 3")
			self.dd:setCooperativeLevel(self.wnd, #DDSCL_NORMAL + #DDSCL_MULTITHREADED)
			--dprint("onActivate hide 4")
			self.wnd:addStyle(#WS_OVERLAPPEDWINDOW)
			--dprint("onActivate hide 5")
			self.wnd:show(#SW_MINIMIZE)
			--dprint("onActivate hide 6")
	

			--dprint("onActivate hide done")
			self.switchModeInProgress = false			

		end)
	elseif activate == 1 then

		if self.switchModeInProgress then return end
		LightThread:new(function()

			self.switchModeInProgress = true
			--dprint("onActivate show")

			self.wnd:removeStyle(#WS_OVERLAPPEDWINDOW)
			self.dd:setCooperativeLevel(self.wnd, #DDSCL_EXCLUSIVE + #DDSCL_FULLSCREEN + #DDSCL_MULTITHREADED)

			if self.w ~= self.currentW or self.h ~= self.currentH then
				self.dd:setMode(self.currentW, self.currentH)
			end

			self.fullscreen_output:create(self.dd)
			self:startRendering()	

			--dprint("onActivate show done")
			self.switchModeInProgress = false			
			self.ignoreMovePosition = false

		end)
	end

end


function RendererHost:startSizing()

	LightThread:new(function()		

		self.newSize = false

		-- dprint("RendererHost:onSize")
		self.switchModeInProgress = true

		self:stopRendering()

		self.window_output:release()
		self.window_output:create(self.dd)

		local w, h = self.window_output:getSize()
		self:sendOnSizeToObjects(w, h)
		local g = self.optionFile:getGroup("video")
		g:set("windowWidth", w)
		g:set("windowHeight", h)

		-- start render thread
		self:startRendering()
		self.switchModeInProgress = false

		-- dprint("RendererHost:onSize done")

		if self.newSize ~= false then
			self:startSizing()
		end

	end)
end

function RendererHost:onMouseMove(x, y)	

	if self.isMouseLeaved then
		self.isMouseLeaved = false
		self.mainQueue:send(Array:new(#MR_SetCursor, #Cursor_Arrow))
		self.wnd:trackMouseLeave()
	end

	self.mx = x
	self.my = y

	self.ro:event(#ROF_ONMOUSEMOVE, x, y)

end

function RendererHost:onMouseButtons(buttons)
	local lstate = buttons % 2

	if self.lstate == 0 then
		if lstate == 1 then
			self.lstate = 1
			self.ro:event(#ROF_LDOWN)
		end
	else
		if lstate == 0 then
			self.lstate = 0
			self.ro:event(#ROF_LUP)
		end
	end

end

function RendererHost:onMouseLeave()
	self.isMouseLeaved = true
end

function RendererHost.getScreenSize()
	return singleton.dd:getScreenSize()
end

function RendererHost.getViewSize()
	local self = singleton
	if self == nil then return 0, 0 end
	local w, h = 0, 0
	if self.mode == #OT_Windowed then
		w, h = self.window_output:getSize()
	elseif self.mode == #OT_Fullscreen then
		w, h = self.fullscreen_output:getSize()
	end
	return w, h
end

function RendererHost.setCursor(id)
	singleton.mainQueue:send(Array:new(#MR_SetCursor, id))
end

function testMode(w, h, rw, rh)
	if  math.floor(h / (w / rw)) == rh then return true end
	return false
end

function RendererHost.getModeList()

	local t = singleton.dd:getModeList()

	local r = {	["4:3"] = { }, ["16:9"] = { }, ["16:10"] = { }, ["unk"] = { } }
	local l = table.getn(t)
	for i = 1, l, 2 do

		local w = t[i]
		local h = t[i + 1]
		local caption = "" .. w .. "x" .. h
		local item = { w = w, h = h, caption = caption }

		local sec = "unk"
		if testMode(w, h, 4, 3) then
			sec = "4:3"
		elseif testMode(w, h, 16, 9) then
			sec = "16:9"
		elseif testMode(w, h, 16, 10) then
			sec = "16:10"
		end			

		table.insert(r[sec], item)

	end
	return r
end

function RendererHost.switchMode(fullscreen, w, h)
	local self = singleton
	if fullscreen then
		if self.mode ~= #OT_Fullscreen or w ~= self.currentW or h ~= self.currentH then
			self:enableFullscreenMode(w, h)
		end
	else
		if self.mode ~= #OT_Windowed then
			self:enableWindowedMode()
		end		
	end
end

--[[
function RendererHost:setWindowPosition(x, y, w, h)
	if x ~= -1 then
		-- dprint("setting coord " .. x .. "x" .. y)
		self.wnd:setPlacement(x, y, w, h)
	end
end
]]

return RendererHost