add_executable(rrfCutTool src/main.cpp inc/argUtilities.hpp)
target_link_libraries(rrfCutTool rrfReaderLib rrfWriterLib)

include_directories(rrfCutTool
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/record/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../../source/components/record/clibs/inc)

SET_TARGET_PROPERTIES(rrfCutTool
        PROPERTIES
        FOLDER tools)

install(TARGETS rrfCutTool RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
