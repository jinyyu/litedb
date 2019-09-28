cmake_minimum_required(VERSION 3.5)
project(kvd)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-Wall)

include_directories(${CMAKE_SOURCE_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(glib glib-2.0 REQUIRED)
include_directories(${glib_INCLUDE_DIRS})
set(LIBS ${glib_LIBRARIES})

add_subdirectory(src)
add_subdirectory(test)