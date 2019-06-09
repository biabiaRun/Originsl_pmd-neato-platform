
############################################
#
# How to build BZLIB
#
############################################

SET(include_HEADERS
  bzlib_private.h
  bzlib.h
)

# Sources and private headers
SET(bzlib_SOURCES
  blocksort.c
  huffman.c
  crctable.c
  randtable.c
  compress.c
  decompress.c
  bzlib.c
)

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

# bz2 is a shared library
ADD_LIBRARY(libbz2 SHARED ${bzlib_SOURCES} ${include_HEADERS})
TARGET_COMPILE_DEFINITIONS(libbz2 PRIVATE _CRT_SECURE_NO_WARNINGS)
SET_TARGET_PROPERTIES(libbz2 PROPERTIES OUTPUT_NAME libbz2 FOLDER contrib/zip)

# bz2static is a static library
ADD_LIBRARY(libbz2static STATIC ${bzlib_SOURCES} ${include_HEADERS})
TARGET_COMPILE_DEFINITIONS(libbz2static PRIVATE _CRT_SECURE_NO_WARNINGS)
SET_TARGET_PROPERTIES(libbz2static PROPERTIES OUTPUT_NAME libbz2static FOLDER contrib/zip)