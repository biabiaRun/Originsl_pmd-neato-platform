if(ROYALE_DEB_PACKAGING)
    install(FILES Getting_Started_CamBoard_pico_flexx.pdf
                  Getting_Started_CamBoard_pico_monstar.pdf
                  RoyaleViewer.pdf
                  DESTINATION "share/doc/royale")
else()
    install(FILES Getting_Started_CamBoard_pico_flexx.pdf
                  Getting_Started_CamBoard_pico_monstar.pdf
                  RoyaleViewer.pdf
                  DESTINATION ".")
endif()
