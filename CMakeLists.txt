# FreeRDP - XPUnlimited Extension
# Copyright (C) 2010 O.S. Systems Software LTDA.

cmake_minimum_required(VERSION 2.6)

project(freerdp-extension-xpunlimited C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

set(CMAKE_COLOR_MAKEFILE ON)

# Include our extra modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

add_subdirectory(src)
