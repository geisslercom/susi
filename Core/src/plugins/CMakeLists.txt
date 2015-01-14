###################################################################################
#
#  Copyright (c) 2015, webvariants GmbH, http://www.webvariants.de
#
#  This file is released under the terms of the MIT license. You can find the
#  complete text in the attached LICENSE file or online at:
#
#  http://www.opensource.org/licenses/mit-license.php
#
#  @author: Tino Rusch (tino.rusch@webvariants.de)
#
###################################################################################

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/heartbeat)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/sessionmanager)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/apiserver)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tcpapiserver)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/webstack)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/dbmanager)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/auth)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/states)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/syscalls)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/duktape)
