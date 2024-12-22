
# author        Oliver Blaser
# date          19.12.2024
# copyright     GPL-3.0 - Copyright (c) 2024 Oliver Blaser

cmake_minimum_required(VERSION 3.13)

project(treesha1sum)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)



#
# SDK
#

# omw
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../sdk/omw/build/cmake/libomw.a/ ${CMAKE_CURRENT_LIST_DIR}/../../sdk/omw/build/cmake/libomw.a/)
include_directories(../../sdk/omw/include)



#
# the application
#

set(BINNAME treesha1sum)

if(_DEBUG)
    add_definitions(-D_DEBUG)
endif()

include_directories(../../src/)

set(SOURCES
../../src/middleware/sha1.cpp
../../src/main.cpp
)



add_executable(${BINNAME} ${SOURCES})
target_link_libraries(${BINNAME} omw)
target_compile_options(${BINNAME} PRIVATE -Wall -Werror=return-type -Werror=switch -Werror=reorder -Werror=format)
