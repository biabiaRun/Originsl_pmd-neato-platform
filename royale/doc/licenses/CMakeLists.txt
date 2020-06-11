file(GLOB LICENSES_BUILDS builds/*.txt)       # Licenses for the build of the package
file(GLOB LICENSES_INTERNAL internal/*.txt)   # Licenses for internal stuff (e.g. tests)
file(GLOB LICENSES_LIBROYALE libroyale/*.txt) # Licenses for the Royale library
file(GLOB LICENSES_PACKAGE package/*.txt)     # Licenses for stuff in the package
file(GLOB LICENSES_ROYALE royale/*.txt)       # Licenses for Royale itself
file(GLOB LICENSES_TOOLS tools/*.txt)         # Licenses for tools in the package

INSTALL(FILES ${LICENSES_LIBROYALE} DESTINATION "doc/licenses/libroyale")
INSTALL(FILES ${LICENSES_PACKAGE} DESTINATION "doc/licenses/package")
INSTALL(FILES ${LICENSES_TOOLS} DESTINATION "doc/licenses/tools")


if(ROYALE_DEB_PACKAGING)
    install(FILES ${PACKAGE_LICENSES} DESTINATION "share/doc/royale")
    install(FILES ${ROYALE_LICENSE_PATH} DESTINATION "share/doc/royale" RENAME "royale_license.txt")
else()
    install(FILES ${PACKAGE_LICENSES} DESTINATION ".")
    install(FILES ${ROYALE_LICENSE_PATH} DESTINATION "." RENAME "royale_license.txt")
endif()

set(ROYALE_VIEWER_LICENSES "${LICENSES_LIBROYALE};${LICENSES_PACKAGE};${LICENSES_TOOLS}")

set(ROYALE_USED_LICENSES_FILE ${CMAKE_CURRENT_BINARY_DIR}/used_licenses.txt CACHE INTERNAL "")
file(WRITE ${ROYALE_USED_LICENSES_FILE} "")
foreach(VIEWER_LICENSEFILE ${ROYALE_VIEWER_LICENSES})
  file(READ ${VIEWER_LICENSEFILE} FILECONTENT)
  file(APPEND ${ROYALE_USED_LICENSES_FILE} "${FILECONTENT}\n\n\n\n")
endforeach()