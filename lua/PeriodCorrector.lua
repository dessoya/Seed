
installModule("thread")

local Object = require("Object")

local PeriodCorrector = Object:extend()

function PeriodCorrector:initialize(periods)

	self.f = queryPerformanceFrequency()
	self.bufferSize = 100

	self.buffer = { }

	self:setPeriodsPerSecond(periods);
end

function PeriodCorrector:setPeriodsPerSecond(periods)

	self.periodPerSecond = periods
	self.msPerPeriod = 1000000.0 / self.periodPerSecond

	self.bufferPos = 1
	self.count = 0
	self.bufferSum = 0
	self.delta = 0

	for i = 1, self.bufferSize do 
		self.buffer[i] = 0
	end

end

function PeriodCorrector:startPeriod()
	self.start = queryPerformanceCounter()
end

function PeriodCorrector:afterDraw()

	local _end = queryPerformanceCounter()

	local c = _end - self.start

	c = c * 1000000.0
	c = c / self.f

	self.msCurrentPeriod = c

end

function PeriodCorrector:endPeriod(skip)

	local _end = queryPerformanceCounter()
	local msCurrentPeriod = _end - self.start

	msCurrentPeriod = msCurrentPeriod * 1000000.0
	msCurrentPeriod = msCurrentPeriod / self.f

	local ms = (self.msPerPeriod - self.msCurrentPeriod - self.delta) / 1000
	self.sleepMS = math.floor(ms)
	self.sleepMS = self.sleepMS - skip
	self.deltaSleep = ms - self.sleepMS

	if self.sleepMS > 0 and skip ~= -1 then
		-- dprint("ms " .. ms)
		sleep(self.sleepMS)
	else 
		self.sleepMS = 0
		threadYield()
	end

	local as = queryPerformanceCounter()

	as = as - self.start

	local msWithSleep = as
	msWithSleep = msWithSleep * 1000000.0
	msWithSleep = msWithSleep / self.f

	self.bufferSum = self.bufferSum - self.buffer[self.bufferPos]
	self.bufferSum = self.bufferSum + msWithSleep

	self.buffer[self.bufferPos] = msWithSleep

	self.bufferPos = self.bufferPos + 1
	if self.bufferPos > self.bufferSize then
		self.bufferPos = 1
	end

	if self.count < self.bufferSize then
		self.count = self.count + 1
	end

	local period = self.bufferSum / self.count

	if period > 0 then
		self.periodsPerSecond = 1000000 / period
	else 
		self.periodsPerSecond = 0
	end

	self.msWithSleep = msWithSleep
	self.delta = period - self.msPerPeriod
end

return PeriodCorrector

