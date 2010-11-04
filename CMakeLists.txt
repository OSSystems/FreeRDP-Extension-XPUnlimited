# FreeRDP - XPUnlimited Extension
# Copyright (C) 2010 O.S. Systems Software LTDA.

cmake_minimum_required(VERSION 2.6)
project(freerdp-extension-xpunlimited)
set(CMAKE_COLOR_MAKEFILE ON)

# Include our extra modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

add_subdirectory(src)

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(DEBUG 1)
endif()
