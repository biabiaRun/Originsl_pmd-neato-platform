find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_INPUT_PATH
        "${ROYALE_SOURCE_DIR}/source/royaleCAPI/inc ${ROYALE_SOURCE_DIR}/source/RoyaleDotNet ${ROYALE_SOURCE_DIR}/source/royale/inc/royale ${ROYALE_SOURCE_DIR}/source/core/inc/royale ${CMAKE_CURRENT_BINARY_DIR}"
        )

    set (DOXYGEN_EXCLUDE_PATTERNS
        "*.m */royaleCAPI/inc/private/* */test_RoyaleDotNet/* */JRoyale/*"
        )

    set(DOXYGEN_OUTPUT_PATH "${CMAKE_BINARY_DIR}/doc/royale")

    file(COPY "${ROYALE_SOURCE_DIR}/README.md" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    configure_file(${ROYALE_SOURCE_DIR}/doc/api/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    add_custom_target(doxygen_api ALL
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        #COMMENT "Generating API documentation with Doxygen" VERBATIM
        )

    SET_TARGET_PROPERTIES(doxygen_api
        PROPERTIES
        FOLDER documentation
        )

    install(DIRECTORY ${DOXYGEN_OUTPUT_PATH}/html_api/
        DESTINATION ${ROYALE_INSTALL_DOC_DIR}/html)
endif(DOXYGEN_FOUND)
