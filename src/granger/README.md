# Granger
[![AppVeyor](https://img.shields.io/appveyor/ci/jkent/granger.svg?style=flat-square)](https://ci.appveyor.com/project/jkent/granger)
[![Travis-CI](https://travis-ci.org/GrangerHub/granger.svg?branch=master "Travis-CI")](https://travis-ci.org/GrangerHub/granger)

### Lua API documentation
`os.access(p, mode)`<br>
Checks that the file is executable/writable/readable. `mode` is a string of the characters "rwx".

`os.chdir(p)`<br>
Change the current working directory.

`os.copyfile(src, dst)`<br>
Copy a file from one location to another.

`os.elevate()`<br>
Attempts to re-run process under elevated privlidges, returning true if elevated or false if unsuccessful.

`os.get()`<br>
Retrieve the current operating system ID string.

`os.getcwd()`<br>
Retrieve the current working directory.

`os.is(id)`<br>
Check the current operating system.

`os.is64bit()`<br>
Determine if the current system is running a 64-bit architecture.

`os.isdir(p)`<br>
Returns true if the specified directory exists. 

`os.isfile(p)`<br>
Returns true if the given file exists.

`os.matchdirs(p)`<br>
Performs a wildcard match to locate one or more directories.

`os.matchfiles(p)`<br>
Performs a wildcard match to locate one or more files.

`os.mkdir(p)`<br>
An overload of os.mkdir() function, which will create any missing subdirectories along the path.

`os.outputof(cmd)`<br>
Run a shell command and return the output.

`os.pathsearch(p, path1, ...)`<br>
Locates a file, given a set of search paths.

`os.rmdir(p)`<br>
Remove a directory, along with any contained files or subdirectories.

`os.stat(p)`<br>
Retrieve information about a file.

`path.getbasename(p)`<br>
Retrieve the filename portion of a path, without any extension.

`path.getabsolute(p)`<br>
Returns an absolute version of a relative path.

`path.getdirectory(p)`<br>
Retrieve the directory portion of a path, or an empty string if the path does not include a directory.

`path.getdrive(p)`<br>
Retrieve the drive letter, if a Windows path.

`path.getextension(p)`<br>
Retrieve the file extension.

`path.getname(p)`<br>
Retreive the filename portion of a path.

`path.getrelative(p1, p2)`<br>
Returns a path relative to another.

`path.isabsolute(p)`<br>
Determines if a path is absolute or relative.

`path.join(...)`<br>
Builds a path from two or more path parts.

`path.normalize(p)`<br>
Removes any wwirdness from a file system path string.

`path.rebase(p, oldbase, newbase)`<br>
Takes a path which is relative to one location and makes it relative to another location instead.

`path.translate(p, sep)`<br>
Convert the separators in a path from one form to another. If `sep` is nil, then a platform-specific separator is used.

`path.wildcards(pattern)`<br>
Converts from a simple wildcard syntax, where * is "match any" and ** is "match recursive", to the corresponding Lua pattern.

`string.explode(s, pattern, plain)`<br>
Returns an array of strings, each which is a substring of `s` formed by splitting on boundaries formed by `pattern`.

`string.endswith(haystack, needle)`<br>
Returns true if `haystack` ends with `needle`.

`string.findlast(s, pattern, plain)`<br>
Find the last instance of a pattern in a string.

`string.startswith(haystack, needle)`<br>
Returns true if `haystack` starts with `needle`.

`table.contains(t, value)`<br>
Returns true if the table contains the specified value.

`table.extract(arr, fname)`<br>
Enumerates an array of objects and returns a new table containing only the value of one particular field.

`table.flatten(arr)`<br>
Flattens a hierarchy of tables into a single array containing all of the values.

`table.implode(arr, before, after, between)`<br>
Merges an array of items into a string.

`table.insertflat(tbl, values)`<br>
Inserts a value of array of values into a table. If the value is itself a table, its contents are enumerated and added instead. So these inputs give these outputs:<br>
"x" -> { "x" }<br>
{ "x", "y" } -> { "x", "y" }<br>
{ "x", { "y" }} -> { "x", "y" }<br>

`table.isempty(t)`<br>
Returns true of the table is empty, and contains no indexed or keyed values.

`table.join(...)`<br>
Adds the values from one array to the end of another and returns the result.

`table.keys(tbl)`<br>
Return a list of all the keys used in a table.

`table.merge(...)`<br>
Adds the key-value associations from one table into another and returns the resulting merged table.

`table.translate(arr, translation)`<br>
Translates the values contained in array, using specified translation table and returns the results in a new array.
