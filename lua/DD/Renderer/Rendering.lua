 
require("AppMessages")
require("DD\\Renderer\\Messages")
installModule("dd_draw_machine")
-- installModule("ordered_array")

local PeriodCorrector = require("PeriodCorrector")

local ThreadPumpChild = require("Thread\\PumpChild")
local Rendering = ThreadPumpChild:extend()

function Rendering:init(control, renderInfoCollectorFile)

	self.control = control
	if renderInfoCollectorFile then
		self.infoCollector = require(renderInfoCollectorFile)
	end

	self.renderingAllow = false
	self.pc = PeriodCorrector:new(60)
	self.drawMachine = DDDrawMachine:new()
end

function Rendering:start()

	self:setMessageNames({
		[#M_Quit]				= "onQuit",
		[#MR_StopRendering]    	= "onStopRendering",
		[#MR_StartRendering]   	= "onStartRendering",
		[#MR_Output]   			= "onOutput"
	})

	self:messageLoop(0)

end

function Rendering:onQuit(returnId)
	self.work = false
	self:send(#MR_Answer, returnId, "rendering")
end

function Rendering:onStopRendering(returnId)
	--dprint("Rendering:onStopRendering " .. returnId)
	self.renderingAllow = false
	if returnId ~= -1 then
		self:send(#MR_Answer, returnId)
	end
end

function Rendering:onStartRendering(returnId)
	self.renderingAllow = true
	if returnId ~= -1 then
		self:send(#MR_Answer, returnId)
	end
end

function Rendering:onOutput(returnId, output, mode)
	self.output = output	
	self.mode = mode
	self:send(#MR_Answer, returnId)
end

function Rendering:beforeReadMessage()

	collectgarbage()

	local pure_rt, dd_rt = 0, 0
	local _start, _end, _one

	if self.renderingAllow then		

		self.pc:startPeriod()

		if self.output:isValid() then

			local drawMachine = self.drawMachine
			-- lprint("1")
			drawMachine:setOutput(self.output)

			if self.infoCollector then
				self.infoCollector(self.pc)
			end

			-- lprint("2")
			self.control:draw(drawMachine)

		end

		self.pc:afterDraw()

		-- lprint("3")
		self.output:flip()
		-- lprint("4")


		if self.mode == "fullscreen" then
			self.pc:endPeriod(-1)
		else
			self.pc:endPeriod(1)
		end

	else
		sleep(3)
	end

end

return Rendering