
-- for commonData

function bind(func, ...)
	local a = { ... }
	-- dump(a)
	return function(...)
		local status, r = pcall(func, unpack(a), ...)
		if status then return r end
		error(r)
	end
end

bn = { [true] = "true", [false] = "false" }

local libs = { }
local preprocessors = { }

local consts, includes = { }, { }

function _parse_macro(lines, file)
	local i, l = 1, table.getn(lines)
	while i <= l do

		local line = lines[i]

		for name in line:gmatch("require%(\"([%a%d%-_\\]+)\"") do
			-- dprint("require '" .. name .. "'")
			-- line = "  "
			-- lines[i] = line

			-- process \\ to \
			name = string.gsub(name, "(\\\\)", "\\")

			if includes[name] == nil then
				local status, result = searchFile(name)
				-- local inc = C_File_Read("lua\\" .. name .. ".lua")
				if status then
					local inc_lines = result:split("\r\n")
					_parse_macro(inc_lines, name)
					includes[name] = true
				else
					error("parse file '" .. file .. ".lua' at line " .. i .. ", error: " .. result)
				end
			end

		end

		for name, value in line:gmatch("#const%s+([%a%d%-_]+)%s+(.+)") do
			-- lprint("found const '" .. name .. "' = '" .. value .."'")
			line = "  "
			lines[i] = line
			consts[name] = value
			break
		end

		while true do

			local count = 0
			for name in line:gmatch("#([%a%d%-_]+)") do
				if consts[name] == nil then
					-- lprint("const '" .. name .. "' not found")
				else
					count = count + 1
					-- lprint("found using const '" .. name .. "'")
					while true do
						local s,e = line:find("#" .. name)
						if s ~= nil then
							line = line:sub(1, s - 1) .. consts[name] .. line:sub(e + 1)
							lines[i] = line
						else
							break
						end
					end
				end						
				break
			end

			if count == 0 then break end
		end

		i = i + 1
	end
end

table.insert(preprocessors, function(lines, file)
	--dprint("macro preprocessors")
	--dprint(type(lines))
	_parse_macro(lines, file)
	return lines
end)

require = function(moduleName)

	assert(type(moduleName) == "string", "moduleName must be string")
	-- dprint(moduleName)

	if not libs[moduleName] then

		-- search file data
		local status, result = searchFile(moduleName)
		if status then

			-- preprocessor
			if table.getn(preprocessors) then
				local lines = result:split("\r\n")
				local i, l = 1, table.getn(preprocessors)
				while i <= l do
					lines = preprocessors[i](lines, moduleName)
					i = i + 1
				end
				result = table.concat(lines, "\r\n")
			end

			-- eval
			local func, err = loadstring(result, moduleName .. ".lua")
			if func == nil then
				libs[moduleName] = true
				error("\r\n" .. err)
			else

				-- execute
				local status, result = pcall(func)
				if status then
					libs[moduleName] = result
				else
					libs[moduleName] = true
					error("\r\n" .. result)
				end

			end

		else
			libs[moduleName] = true
			error("\r\n" .. result)
		end
	end

	return libs[moduleName]

end

installModule("array")
require("CommonData")

local _formatscalar = function(v)
	if type(v) == "string" then
		return v .. ":stirng"
	elseif type(v) == "number" then
		return "" .. v .. ":number"
	elseif type(v) == "boolean" then
		if v then
			return "true:boolean"
		end
		return "false:boolean"
	elseif type(v) == "function" then
		return "function"
	else
		return "unk:" .. type(v)
	end
	return v
end

dump = function(v, level)

	if level == nil then level = 0 end
	local align = ""
	for i = 1, level do
		align = align .. "  "
	end


	if type(v) == "table" then
		for key, val in pairs(v) do
			if type(val) == "table" then
				dprint(align .. key .. " = table")
				dump(val, level + 1)
			else
				dprint(align .. key .. " = " .. _formatscalar(val))
			end
		end
	else
		dprint(align .. _formatscalar(v))
	end
end

dofile = nil
loadfile = nil
load = nil
-- loadstring = nil
print = nil

