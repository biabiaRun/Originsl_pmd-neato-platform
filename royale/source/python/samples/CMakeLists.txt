configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/activation_code.py" "${CMAKE_CURRENT_BINARY_DIR}/activation_code.py")

add_custom_target(copy_python_samples ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_BINARY_DIR}/activation_code.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/activation_code.py"
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/sample_camera_info.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/sample_camera_info.py"
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/sample_export_ply.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/sample_export_ply.py"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/sample_opencv.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/sample_opencv.py"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/sample_processing_parameters.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/sample_processing_parameters.py"
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/sample_rawdata.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/sample_rawdata.py"
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/sample_record_rrf.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/sample_record_rrf.py"
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/sample_retrieve_data.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/sample_retrieve_data.py"
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/sample_retrieve_depth_image.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/sample_retrieve_depth_image.py"
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/roypy_sample_utils.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/roypy_sample_utils.py"
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_CURRENT_SOURCE_DIR}/roypy_platform_utils.py" 
    "${ROYALE_RUNTIME_OUTPUT_DIRECTORY}/roypy_platform_utils.py"
    
    COMMENT "Copying Python samples" VERBATIM
    )
    
set_target_properties(copy_python_samples
   PROPERTIES
   FOLDER python
   )    
   
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/sample_camera_info.py"
    DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/sample_export_ply.py"
    DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/sample_opencv.py"
    DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/sample_record_rrf.py"
    DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/sample_retrieve_data.py"
    DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/sample_retrieve_depth_image.py"
    DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})        
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/roypy_sample_utils.py"
    DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/roypy_platform_utils.py"
    DESTINATION ${ROYALE_INSTALL_PYTHON_DIR})
   