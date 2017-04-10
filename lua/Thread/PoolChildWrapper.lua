
installModule("thread")
installModule("array")

function thread(a)

	local filename, index, ownerQueue, poolQueue, childQueue = a:get(0, 5, true)
	local r = require(filename)
	local o = r:new(index, ownerQueue, poolQueue, childQueue, a:get(5, -1, true))
	o:start()

end
