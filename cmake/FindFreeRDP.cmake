# FindFreeRDP - Find FreeRDP on system
#
# This finds the FreeRDP headers and libraries doing the required
# checks to allow for easy deployment of FreeRDP extensions and
# plugins.
#
#     Copyright (C) 2010 O.S. Systems Software LTDA.
#     Copyright (C) 2010 Otavio Salvador <otavio@ossystems.com.br>

# Following macros has been defined:
#
#  freerdp_add_extension(NAME SOURCES...)
#    This creates an extension and will be installed inside of lib/freerdp/extensions/

# All the libraries required are stored in a variable called FREERDP_LIBRARIES.
# Add this variable to your TARGET_LINK_LIBRARIES.

macro(freerdp_add_extension NAME SOURCES)
  add_library(${NAME} MODULE ${SOURCES} ${ARGN})
  set_target_properties(${NAME} PROPERTIES PREFIX "")
  install(TARGETS ${NAME} DESTINATION lib/freerdp/extensions)
endmacro()

set(FREERDP_REQUIRED_LIBRARIES
  freerdp-core)

find_path(FREERDP_INCLUDE_DIR
          NAMES freerdp.h
		  PATH_SUFFIXES freerdp
		  DOC "FreeRDP headers path")

foreach(lib ${FREERDP_REQUIRED_LIBRARIES})
  find_library(FREERDP_LIB_${lib} NAMES ${lib})

  if(NOT FREERDP_LIB_${lib})
	message(FATAL_ERROR "Could not find FreeRDP library '${lib}'")
  endif()

  mark_as_advanced(${FREERDP_LIB_${lib}})
endforeach()

mark_as_advanced(FREERDP_INCLUDE_DIR)

foreach(lib ${FREERDP_REQUIRED_LIBRARIES})
  set(FREERDP_LIBRARIES ${FREERDP_LIBRARIES} ${FREERDP_LIB_${lib}})
endforeach()

set(FREERDP_INCLUDES ${FREERDP_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeRDP DEFAULT_MSG FREERDP_INCLUDES FREERDP_LIBRARIES)
