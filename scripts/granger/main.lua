--
-- main.lua
-- Granger main
-- Copyright (c) 2016 Jeff Kent <jeff@jkent.net>
--

require 'lib'

local src-dir = os.getenv('GRANGER_SRC')
if not src_dir then
	os.exit()
end

local dst_dir = path.getdirectory(_EXE_PATH)
local tremulous_exe = "tremulous"
if os.is('windows') then
	tremulous_exe = tremulous_exe .. '.exe'
end

local src = path.join(src_dir, tremulous_exe)
local dst = path.join(dst_dir, tremulous_exe)

if not os.access(dst, 'w') then
	os.elevate()
end

os.rename(src, dst)
