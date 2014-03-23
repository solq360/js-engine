MESSAGE(STATUS "Using bundled Findlibm.cmake...")
FIND_PATH(
	LIBM_INCLUDE_DIR
	math.h
	/usr/include
	/usr/local/include
)


FIND_LIBRARY(
	LIBM_LIBRARIES NAMES m
	PATHS /usr/lib/ /usr/local/lib/
)