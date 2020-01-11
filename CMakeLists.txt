cmake_minimum_required(VERSION 3.5)
project(litedb)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-Wall)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/3rd/)

set(LIBS ${LIBS} pthread)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(glib glib-2.0 REQUIRED)
include_directories(${glib_INCLUDE_DIRS})
set(LIBS ${LIBS} ${glib_LIBRARIES})

pkg_check_modules(lmdb lmdb REQUIRED)
include_directories(${lmdb_INCLUDE_DIRS})
set(LIBS ${LIBS} ${lmdb_LIBRARIES})

add_subdirectory(src)
add_subdirectory(test)