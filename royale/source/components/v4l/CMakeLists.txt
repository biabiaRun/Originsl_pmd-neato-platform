set (V4L_SOURCES
    "src/BridgeV4l.cpp"
    )

set (V4L_HEADERS
    "inc/royalev4l/bridge/BridgeV4l.hpp"
    )

add_library (component_v4l OBJECT ${V4L_SOURCES} ${V4L_HEADERS})

set_target_properties(component_v4l
    PROPERTIES
    FOLDER components
    EXCLUDE_FROM_ALL true
    )

add_subdirectory(test)
