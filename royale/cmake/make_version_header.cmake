#****************************************************************************
# Copyright (C) 2018 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

#
# This is a bit complicated, so here's an explanation on what it does,
# and why:
# - During cmake invocation, a script (update-royale-version.sh) is
#   generated, containing the royale version number and the git
#   command for getting the SCM revision.
#   This needs to be a script because it needs to be invoked from a
#   build rule; the "automatic" cmake invocation at the start of the
#   build isn't triggered in all situations where the SCM revision
#   might have changed (at least with Makefile based generators; for
#   MSVC generators it might be sufficient as they seem to always run
#   cmake during the build).
#   Having the royale version in the script works because the royale
#   version is defined in the top-level CMakeLists.txt; any change
#   there would trigger a cmake run which regenerates
#   update-royale-version.sh.
# - To avoid unnecessary rebuilds, the update-royale-version.sh script
#   writes it's output to a temporary file first and only touches
#   royale-version.h if there is any change (or it didn't exist
#   before); it uses a helper script (move-if-change) for this.
# - In every CMakeLists.txt with sources using royale-version.h, a
#   custom command needs to be added that depends on
#   update-royale-version and lists royale-version.h as output.
#   Now cmake should (as claimed by the documentation) be able to
#   detect the dependency on the generated file (even if it doesn't
#   exist yet); however this doesn't seem to work reliably so the
#   dependency has to be added explicitly.
#   A macro (DEPENDS_ON_ROYALE_VERSION_H) is provided which does both;
#   it expects a list of .cpp files which include royale-version.h as
#   argument.
#
#


# Need git and bash
ROYALE_FIND_HOST_PACKAGE(UnixCommands)
ROYALE_FIND_HOST_PACKAGE(Git)

# UnixCommands isn't REQUIRED because that would also includes things
# like tar (which we can live without).
# However we still require BASH.
if(${BASH} STREQUAL "BASH-NOTFOUND")
  if(CMAKE_HOST_WIN32)
    get_filename_component(GitPath ${GIT_EXECUTABLE} DIRECTORY)
    string(REGEX REPLACE "Git/cmd" "Git/bin" GitPath2 ${GitPath})
    find_program(BASH 
    NAMES bash
    PATHS "${GitPath}" "${GitPath2}"
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH)
    if(${BASH} STREQUAL "BASH-NOTFOUND")
      message(FATAL_ERROR "No bash found.")
    endif()
  else()
    message(FATAL_ERROR "No bash found.")
  endif()
endif()

# Remove old instances of the royale-version header
file(GLOB_RECURSE OLD_VERSION_HEADERS "${CMAKE_BINARY_DIR}/royale-version.h")
FOREACH(FILE ${OLD_VERSION_HEADERS})
    file(REMOVE ${FILE})
ENDFOREACH(FILE)    

# Windows 10 / WSL specific handling
set(BASH_COMMAND "${BASH}")
if (WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
	get_filename_component(BASH_DIR "${BASH}" DIRECTORY)
	get_filename_component(SYS32_DIR "$ENV{WINDIR}/System32" ABSOLUTE)
	if (${BASH_DIR} STREQUAL ${SYS32_DIR})
		set(BASH_COMMAND "$ENV{WINDIR}/Sysnative/wsl.exe")
	endif()
endif()

# Macro which creates the proper dependencies for source files
# that include royale-version.h.
macro(DEPENDS_ON_ROYALE_VERSION_H)
  # Create script from template.
  # This is run during cmake invocation.
  configure_file(
    ${ROYALE_SOURCE_DIR}/cmake/update-royale-version.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/update-royale-version.sh
    @ONLY
    NEWLINE_STYLE UNIX
    )
  SET(ROYALE_VERSION_H ${CMAKE_CURRENT_BINARY_DIR}/royale-version.h)
  if(NOT TARGET "${ROYALE_VERSION_H}")
    add_custom_command(OUTPUT 
      "${ROYALE_VERSION_H}"
      "${CMAKE_CURRENT_BINARY_DIR}/__royale-version.h"
      COMMAND ${BASH_COMMAND} ./update-royale-version.sh
      COMMENT "Running update-royale-version.sh"
	  VERBATIM
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      )
  endif()
  set_source_files_properties(${ARGV}
    # this one is always missing so we're running every time
    PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/__royale-version.h"
    )
endmacro()
