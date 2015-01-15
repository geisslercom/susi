cmake_minimum_required(VERSION 2.8)

project (duktape)
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "--std=c++11 -g -Wno-return-type-c-linkage ${CMAKE_CXX_FLAGS}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)
include_directories(~/Code/susi/Core/src/headers)

add_library(duktape SHARED sources/export.cpp
						   sources/DuktapeEngine.cpp
						   sources/SusiJS.cpp
						   sources/duktape.cpp)
