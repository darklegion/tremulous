if(NOT WIN32)
    add_definitions(-Wall -Wextra)
endif(NOT WIN32)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WIN32)

add_library(granger_nettle
    macros.h
    md5-compress.c
    md5.c
    md5.h
    nettle-stdint.h
    nettle-types.h
    nettle-write.h
    sha2.h
    sha256-compress.c
    sha256.c
    version.h
    write-be32.c
    write-le32.c
    )
