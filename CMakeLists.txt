cmake_minimum_required(VERSION 3.20)
project(WindowsEdgeLightNative)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set subsystem to Windows (GUI app)
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")
    # Use static runtime for smaller executable
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Add source files
add_executable(WindowsEdgeLightNative WIN32
    main.cpp
    WindowsEdgeLightNative.rc
)

# Link libraries
target_link_libraries(WindowsEdgeLightNative
    d2d1
    dwrite
    windowscodecs
    comctl32
)

# Set output directory
set_target_properties(WindowsEdgeLightNative PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin"
)
