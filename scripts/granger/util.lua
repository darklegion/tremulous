--
-- util.lua
-- various utility functions
-- Copyright (c) 2016 Jeff Kent <jeff@jkent.net>
--

local function hash_file(file, ctx)
	local f = io.open(file, "r")
	if f == nil then
		return nil
	end
	repeat
		local buf = f:read(0x10000)
		ctx:update(buf)
	until buf == nil
	f:close()
	return tostring(ctx)
end

function sha256_file(file)
	local ctx = nettle.sha256()
	return hash_file(file, ctx)
end

function md5_file(file)
	local ctx = nettle.md5()
	return hash_file(file, ctx)
end
