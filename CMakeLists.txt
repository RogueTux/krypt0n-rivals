cmake_minimum_required(VERSION 3.10)
project(krypt0n)

set(OpenGL_GL_PREFERENCE GLVND)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-DGLEW_STATIC)

# Directory definitions
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# ImGui source files
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(FEATURE_SOURCES
        ${SRC_DIR}/EntityScanner.cpp
        ${SRC_DIR}/AccuracyAssist.cpp
        ${SRC_DIR}/GameData.cpp
        ${SRC_DIR}/Memory.cpp
        ${SRC_DIR}/Overlay.cpp
        ${SRC_DIR}/ProcessUtils.cpp
        ${SRC_DIR}/Utils.cpp
        ${SRC_DIR}/FeatureManager.cpp
        ${SRC_DIR}/MouseControl.cpp
)

# UI source files
set(UI_SOURCES
        ${SRC_DIR}/main.cpp
        ${SRC_DIR}/menu.cpp
        ${SRC_DIR}/theme.cpp
        ${SRC_DIR}/tabs/dashboard.cpp
        ${SRC_DIR}/tabs/visuals.cpp
        ${SRC_DIR}/tabs/aim.cpp
        ${SRC_DIR}/tabs/misc.cpp
        ${SRC_DIR}/tabs/settings.cpp
)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(X11 REQUIRED)
find_library(XFIXES_LIBRARY Xfixes REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)

# Create executable
add_executable(${PROJECT_NAME}
        ${FEATURE_SOURCES}
        ${UI_SOURCES}
        ${IMGUI_SOURCES}
)

# Include directories (Add libevdev includes)
target_include_directories(${PROJECT_NAME} PRIVATE
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${INCLUDE_DIR}
        ${SRC_DIR}
        ${SRC_DIR}/tabs
        ${OPENGL_INCLUDE_DIRS}
        ${GLFW_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${X11_INCLUDE_DIR}
        ${LIBEVDEV_INCLUDE_DIRS}
)

# Link libraries (Add libevdev, remove Xtst)
target_link_libraries(${PROJECT_NAME} PRIVATE
        OpenGL::GL
        glfw
        GLEW::GLEW
        X11
        ${XFIXES_LIBRARY}
        ${LIBEVDEV_LIBRARIES}
        pthread
)

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)