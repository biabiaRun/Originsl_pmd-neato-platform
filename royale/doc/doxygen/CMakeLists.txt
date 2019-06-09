ROYALE_FIND_HOST_PACKAGE(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_INPUT_PATH "${ROYALE_SOURCE_DIR}/source")
    set(DOXYGEN_OUTPUT_PATH "${CMAKE_BINARY_DIR}/doc")
    configure_file(${ROYALE_SOURCE_DIR}/doc/doxygen/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    add_custom_target(doxygen ALL
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
    
    SET_TARGET_PROPERTIES(doxygen
        PROPERTIES
        FOLDER documentation
        )   
endif(DOXYGEN_FOUND)
