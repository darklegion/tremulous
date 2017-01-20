--
-- os.lua
-- Additions to the OS namespace.
-- Copyright (c) 2002-2011 Jason Perkins and the Premake project
-- Copyright (c) 2015 Jeff Kent <jeff@jkent.net>
--


--
-- Retrieve the current operating system ID string.
--

	function os.get()
		return _OS
	end



--
-- Check the current operating system.
--

	function os.is(id)
		return (os.get():lower() == id:lower())
	end



--
-- Determine if the current system is running a 64-bit architecture
--

	local _64BitHostTypes = {
		"x86_64",
		"ia64",
		"amd64",
		"ppc64",
		"powerpc64",
		"sparc64"
	}

	function os.is64bit()
		-- Call the native code implementation. If this returns true then
		-- we're 64-bit, otherwise do more checking locally
		if (os._is64bit()) then
			return true
		end

		-- Identify the system
		local arch
		if _OS == "windows" then
			arch = os.getenv("PROCESSOR_ARCHITECTURE")
		elseif _OS == "macosx" then
			arch = os.outputof("echo $HOSTTYPE")
		else
			arch = os.outputof("uname -m")
		end

		-- Check our known 64-bit identifiers
		arch = arch:lower()
		for _, hosttype in ipairs(_64BitHostTypes) do
			if arch:find(hosttype) then
				return true
			end
		end
		return false
	end



--
-- The os.matchdirs() and os.matchfiles() functions
--

	local function domatch(result, mask, wantfiles)
		-- need to remove extraneous path info from the mask to ensure a match
		-- against the paths returned by the OS. Haven't come up with a good
		-- way to do it yet, so will handle cases as they come up
		if mask:startswith("./") then
			mask = mask:sub(3)
		end

		-- strip off any leading directory information to find out
		-- where the search should take place
		local basedir = mask
		local starpos = mask:find("%*")
		if starpos then
			basedir = basedir:sub(1, starpos - 1)
		end
		basedir = path.getdirectory(basedir)
		if (basedir == ".") then basedir = "" end

		-- recurse into subdirectories?
		local recurse = mask:find("**", nil, true)

		-- convert mask to a Lua pattern
		mask = path.wildcards(mask)

		local function matchwalker(basedir)
			local wildcard = path.join(basedir, "*")

			-- retrieve files from OS and test against mask
			local m = os.matchstart(wildcard)
			while (os.matchnext(m)) do
				local isfile = os.matchisfile(m)
				if ((wantfiles and isfile) or (not wantfiles and not isfile)) then
					local basename = os.matchname(m)
					local fullname = path.join(basedir, basename)
					if basename ~= ".." and fullname:match(mask) == fullname then
						table.insert(result, fullname)
					end
				end
			end
			os.matchdone(m)

			-- check subdirectories
			if recurse then
				m = os.matchstart(wildcard)
				while (os.matchnext(m)) do
					if not os.matchisfile(m) then
						local dirname = os.matchname(m)
						if (not dirname:startswith(".")) then
							matchwalker(path.join(basedir, dirname))
						end
					end
				end
				os.matchdone(m)
			end
		end

		matchwalker(basedir)
	end

	function os.matchdirs(...)
		local result = { }
		for _, mask in ipairs(arg) do
			domatch(result, mask, false)
		end
		return result
	end

	function os.matchfiles(...)
		local result = { }
		for _, mask in ipairs(arg) do
			domatch(result, mask, true)
		end
		return result
	end



--
-- An overload of the os.mkdir() function, which will create any missing
-- subdirectories along the path.
--

	local builtin_mkdir = os.mkdir
	function os.mkdir(p)
		local dir = iif(p:startswith("/"), "/", "")
		for part in p:gmatch("[^/]+") do
			dir = dir .. part

			if (part ~= "" and not path.isabsolute(part) and not os.isdir(dir)) then
				local ok, err = builtin_mkdir(dir)
				if (not ok) then
					return nil, err
				end
			end

			dir = dir .. "/"
		end

		return true
	end


--
-- Run a shell command and return the output.
--

	function os.outputof(cmd)
		local pipe = io.popen(cmd)
		local result = pipe:read('*a')
		pipe:close()
		return result
	end


--
-- Remove a directory, along with any contained files or subdirectories.
--

	local builtin_rmdir = os.rmdir
	function os.rmdir(p)
		-- recursively remove subdirectories
		local dirs = os.matchdirs(p .. "/*")
		for _, dname in ipairs(dirs) do
			os.rmdir(dname)
		end

		-- remove any files
		local files = os.matchfiles(p .. "/*")
		for _, fname in ipairs(files) do
			os.remove(fname)
		end

		-- remove this directory
		builtin_rmdir(p)
	end


--
-- Elevate and set _ELEVATED global
--

	_ENV._ELEVATED = false
	local builtin_elevate = os.elevate
	function os.elevate()
		_ENV._ELEVATED = builtin_elevate()
		return _ENV._ELEVATED
	end

