--
-- main.lua
-- test runner
-- Copyright (c) 2016 Jeff Kent <jeff@jkent.net>
--

package.path = package.path .. ";../lua/?.lua;../lua/?/init.lua"

require "lib"
require "test-os-access"
require "test-nettle"
