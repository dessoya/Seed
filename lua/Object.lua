
local Object = {}
Object.meta = {__index = Object}

function Object:_create()
  local meta = rawget(self, "meta")
  if not meta then error("Cannot inherit from instance object") end
  return setmetatable({}, meta)
end

function Object:new(...)
  local obj = self:_create()
  if type(obj.initialize) == "function" then
    obj:initialize(...)
  end
  return obj
end


function Object:extend()
  local obj = self:_create()
  local meta = {}
  for k, v in pairs(self.meta) do
    meta[k] = v
  end
  meta.__index = obj
  meta.super=self
  obj.meta = meta
  return obj
end


return Object