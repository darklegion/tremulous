if(NOT WIN32)
    add_definitions(-Wall -Wextra)
endif(NOT WIN32)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WIN32)

add_library(premake
	os_access.c
	os_chdir.c
	os_copyfile.c
	os_elevate.c
	os_getcwd.c
	os_is64bit.c
	os_isdir.c
	os_isfile.c
	os_match.c
	os_mkdir.c
	os_pathsearch.c
	os_rmdir.c
	os_stat.c
	path_getabsolute.c
	path_getrelative.c
	path_isabsolute.c
	path_join.c
	path_normalize.c
	path_translate.c
	premake.c
	premake.h
	string_endswith.c
    )
