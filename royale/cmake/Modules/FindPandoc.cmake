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
# Find pandoc
#

find_program(PANDOC_EXECUTABLE NAMES pandoc)

if(PANDOC_EXECUTABLE)
  foreach(component ${Pandoc_FIND_COMPONENTS})
    if(component STREQUAL "LATEX_ENGINE_OPT")
      execute_process(COMMAND ${PANDOC_EXECUTABLE} --help
            OUTPUT_VARIABLE _TMP_HELP
             ERROR_QUIET
      )
	  execute_process(COMMAND ${PANDOC_EXECUTABLE} --version
            OUTPUT_VARIABLE _TMP_VERSION
             ERROR_QUIET
      )
      string(REGEX MATCHALL "^[^0-9]*([0-9]+\\.[0-9]+\\.[0-9]+).*$" PANDOC_VERSION ${_TMP_VERSION})
      set(PANDOC_VERSION ${CMAKE_MATCH_1})
      if(_TMP_HELP MATCHES "--latex-engine-opt")
        set(Pandoc_LATEX_ENGINE_OPT_FOUND TRUE)
      endif()
    else()
      message(FATAL_ERROR "Pandoc component ${component} unknown")
    endif()
  
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Pandoc
   FOUND_VAR PANDOC_FOUND
   REQUIRED_VARS PANDOC_EXECUTABLE
   VERSION_VAR PANDOC_VERSION
   HANDLE_COMPONENTS
   )
