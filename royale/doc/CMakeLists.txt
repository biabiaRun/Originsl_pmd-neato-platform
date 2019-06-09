add_subdirectory(licenses)
add_subdirectory(getting_started)
add_subdirectory(natvis)

if (ROYALE_ENABLE_DOC)
  add_subdirectory(doxygen)
  add_subdirectory(api)

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/API_Documentation.html ${CMAKE_CURRENT_BINARY_DIR}/API_Documentation.html)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/API_Documentation.html DESTINATION ".")
endif()
