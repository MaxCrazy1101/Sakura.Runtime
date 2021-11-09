set(EASTL_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/EASTL CACHE STRING INTERNAL FORCE)
set(EASTL_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/EASTL CACHE STRING INTERNAL FORCE)

#file(GLOB_RECURSE eastl_sources ${EASTL_SOURCES_DIR}/*.cpp ${EASTL_SOURCES_DIR}/*.c)
set(eastl_sources ${EASTL_SOURCES_DIR}/EASTL/eastl.cpp)
file(GLOB_RECURSE eastl_headers ${EASTL_INCLUDES_DIR}/*.h ${EASTL_INCLUDES_DIR}/*.hpp)

set(EASTL_SOURCES ${eastl_sources} CACHE STRING INTERNAL FORCE)
set(EASTL_HEADERS ${eastl_headers} CACHE STRING INTERNAL FORCE)