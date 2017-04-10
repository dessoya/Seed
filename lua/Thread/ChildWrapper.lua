
installModule("thread")
installModule("array")

function thread(a)

	local filename, ownerQueue, childQueue = a:get(0, 3, true)
	local r = require(filename)
	local o = r:new(0, ownerQueue, nil, childQueue, a:get(3, -1, true))
	o:start()

end