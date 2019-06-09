INCLUDE(${ROYALE_SOURCE_DIR}/cmake/FindMATLABMCR.cmake)

IF(NOT Matlab_FOUND)
    RETURN()
ENDIF()    

INCLUDE_DIRECTORIES(
    ${Matlab_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../royale/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/inc 	
    )

LINK_DIRECTORIES(
    ${PROCESSING_SPECTRE_LIB_DIR}
    )

set(RC_PRODUCTNAME "royale_matlab_wrapper")
set(RC_FILETYPE "VFT_DLL")
set(RC_FILEDESCRIPTION "Matlab wrapper for Royale")
set(RC_INTERNALNAME "royale_matlab")
set(RC_ORIGINALFILENAME "royale_matlab")
CREATE_ASSEMBLY_RC_FILE(ASSEMBLY_RC)   
    
ADD_LIBRARY(royale_matlab MODULE 
    "${CMAKE_CURRENT_SOURCE_DIR}/royale_matlab.cpp"
    ${ASSEMBLY_RC}
    )
ADD_DEFINITIONS(-DMATLAB_MEX_FILE)

# Needed for entry point.
if(WIN32)
SET_TARGET_PROPERTIES(royale_matlab
    PROPERTIES
	DEBUG_POSTFIX ""
    LINK_FLAGS "/export:mexFunction"
    FOLDER modules
)
endif()

# Change the dll extension to a mex extension.
SET_TARGET_PROPERTIES(royale_matlab PROPERTIES SUFFIX ".${Matlab_MEX_EXTENSION}")
SET_TARGET_PROPERTIES(royale_matlab PROPERTIES PREFIX "")

TARGET_LINK_LIBRARIES(royale_matlab royale ${Matlab_LIBRARIES})

install(TARGETS royale_matlab
            RUNTIME DESTINATION ${ROYALE_INSTALL_MATLAB_DIR}
            LIBRARY DESTINATION ${ROYALE_INSTALL_MATLAB_DIR}
        )

SET(ROYALE_INSTALL_TO_MATLAB_FOLDER ON CACHE BOOL "" FORCE)

MARK_AS_ADVANCED(ROYALE_INSTALL_TO_MATLAB_FOLDER)
