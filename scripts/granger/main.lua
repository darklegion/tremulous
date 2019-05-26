--
-- main.lua
-- Granger main
-- Copyright (c) 2016 Jeff Kent <jeff@jkent.net>
--

require 'scripts/granger/lib'

local install_files = {}

if os.is('windows') then
    install_files = {
        "tremulous.exe"
        "tremded.exe"
        "granger.exe"
        "SDL264.dll"
        "renderer_opengl1.dll"
        "renderer_opengl2.dll"
    }
elseif os.is('linux') then
    install_files = {
        "tremulous"
        "tremded"
        "granger"
        "renderer_opengl1.so"
        "renderer_opengl2.so"
    }
elseif os.is('macosx') then
    install_files = {
        "tremulous"
        "tremded"
        "granger"
        "libSDL2-2.0.0.dylib"
        "renderer_opengl1.dylib"
        "renderer_opengl2.dylib"
    }
else 
    os.exit(1)
end

local dst_dir = path.getdirectory('.')
local dst_dir = path.getdirectory(_EXE_PATH)

local privs = false
for file in ipairs(install_files) do
    local src = path.join(src_dir, file)
    local dst = path.join(dst_dir, file)
    if not os.access(dst, 'w') then
        privs = true
    end
end

if privs then
    os.elevate()
end

for file in ipairs(install_files) do
    local src = path.join(src_dir, file)
    local dst = path.join(dst_dir, file)
    os.rename(src, dst)
end

--- Copyright (C) 2015-2019 GrangerHub
