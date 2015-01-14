cmake_minimum_required(VERSION 2.8)

project (dbmanager)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "--std=c++11 -g -Wno-return-type-c-linkage ${CMAKE_CXX_FLAGS}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)
include_directories(~/Code/susi/Core/src/headers)
include_directories(/usr/local/include/soci)

add_library(dbmanager SHARED sources/export.cpp 
							 sources/DBComponent.cpp
							 sources/DBManagerItem.cpp
							 sources/Database.cpp
							 sources/Manager.cpp)
