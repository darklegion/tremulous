--
-- test-os-access.lua
-- test case for os.access()
-- Copyright (c) 2016 Jeff Kent <jeff@jkent.net>
--

print "os.access() test begin"

if not os.is("windows") then
    p = os.tmpname()
	os.execute("touch " .. p .. "; chmod 400 " .. p)
	assert(os.access(p, "r") == true)
	assert(os.access(p, "w") == false)
	os.execute("rm -f " .. p)
end

print "os.access() test end"
