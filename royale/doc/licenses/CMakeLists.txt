SET(THIRD_PARTY_LICENSES_WITHOUT_CYPRESS
    gpl-3.0.txt 
    lgpl-2.1.txt 
    lgpl-3.0.txt 
    jquery_license.txt 
    sizzle_license.txt 
    gradle-wrapper_license.txt 
    kissfft_license.txt 
    packedarray_license.txt
    )

IF(${ROYALE_PLAYBACK_ONLY})
    SET(PACKAGE_LICENSES
        ${THIRD_PARTY_LICENSES_WITHOUT_CYPRESS}
        )
ELSE()
    SET(PACKAGE_LICENSES
        ${THIRD_PARTY_LICENSES_WITHOUT_CYPRESS} 
        cypress_license.txt 
        )
ENDIF()    

INSTALL(FILES ${PACKAGE_LICENSES} DESTINATION ".")
INSTALL(FILES ${ROYALE_LICENSE_PATH} DESTINATION "." RENAME "royale_license.txt")
