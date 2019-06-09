find_package(Threads REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/royale/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/inc
)

set(SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
  )

link_directories(
	${PROCESSING_SPECTRE_LIB_DIR}
    )

add_executable(det
  ${SOURCES}
  )

target_link_libraries(det royale ${CMAKE_THREAD_LIBS_INIT})

SET_TARGET_PROPERTIES(det
    PROPERTIES
    FOLDER tools
    )

install(TARGETS det RUNTIME DESTINATION ${ROYALE_INSTALL_BIN_DIR} COMPONENT DevPack OPTIONAL EXCLUDE_FROM_ALL)
