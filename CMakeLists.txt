# Copyright (c) 2021, A Effective Infrastructure Committee.
# All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
cmake_minimum_required(VERSION 3.14)

if(DEFINED PROJECT_NAME)
  set(subproject ON)
else()
  set(subproject OFF)
endif()

project(
  logxx
  VERSION 0.1.0
  LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(NOT subproject)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

add_subdirectory(third_party)

set(LOGXX_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(LOGXX_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

add_subdirectory(src)

add_subdirectory(test)
