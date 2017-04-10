
local a = commonData()

_get = function(index, count)
	if count == nil then count = 1 end	
	return a:get(index, count, true)
end

_set = function(...)
	a:set(...)
end