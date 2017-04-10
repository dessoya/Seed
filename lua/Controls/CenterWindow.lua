
local Control = require("Control")
local CenterWindow = Control.makeWrap(RBox)

CenterWindow:addEvents(Control.elibs.centring)

return CenterWindow