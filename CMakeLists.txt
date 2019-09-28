cmake_minimum_required(VERSION 3.5)
project(liteSQL)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-Wall)

include_directories(${CMAKE_SOURCE_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(glib glib-2.0 REQUIRED)
include_directories(${glib_INCLUDE_DIRS})
set(LIBS ${glib_LIBRARIES})


find_package(Boost REQUIRED
             COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS
    ${LIBS}
    pthread Boost::system Boost::filesystem)

add_subdirectory(src)
add_subdirectory(test)