cmake_minimum_required(VERSION 3.21)

project(PointCloudApp LANGUAGES CXX)

# ---------------------------
# C++ Standard
# ---------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------
# Enable Qt automoc/uic/rcc
# ---------------------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---------------------------
# Source Files
# ---------------------------

file(GLOB_RECURSE VOLTENGINE_SOURCE
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/../VoltEngine/source/*.cpp
)

list(REMOVE_ITEM VOLTENGINE_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/../VoltEngine/source/main.cpp
)

add_library(VoltEngine OBJECT ${VOLTENGINE_SOURCE}
        ../VoltEngine/source/Components/Square.h)

file(GLOB_RECURSE PROJECT_SOURCES
    source/*.cpp
    source/*.c
)

file(GLOB_RECURSE PROJECT_HEADERS
    source/*.h
    source/*.hpp
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

# ---------------------------
# Find Packages
# ---------------------------

# Vulkan
find_package(Vulkan REQUIRED)

# Qt6 (change to Qt5 if needed)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# GLM (header-only)
target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibraries/glm
)

target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibraries/tiny-gltf)

target_include_directories(VoltEngine
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibraries/glm
)

target_include_directories(VoltEngine
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibraries/tiny-gltf)

# ---------------------------
# Include Directories
# ---------------------------

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# ---------------------------
# STB (header-only)
# ---------------------------
# Assumes stb headers are somewhere under source/
# If stb is in source/thirdparty/stb, this already works
# If not, adjust the path below:

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibraries/stb-master
)

target_include_directories(VoltEngine
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibraries/stb-master
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../VoltEngine/source
)

target_include_directories(VoltEngine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../VoltEngine/source
)

target_link_libraries(VoltEngine
        PRIVATE
        Vulkan::Vulkan
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        TINYGLTF3_ENABLE_FS
)

# ---------------------------
# Onnx
# ---------------------------

set(ONNXRUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibraries/onnxruntime-linux-x64-1.28.0")

add_library(onnxruntime SHARED IMPORTED)

set_target_properties(onnxruntime PROPERTIES
        IMPORTED_LOCATION
        "${ONNXRUNTIME_ROOT}/lib/libonnxruntime.so"
        INTERFACE_INCLUDE_DIRECTORIES
        "${ONNXRUNTIME_ROOT}/include"
)

# ---------------------------
# Link Libraries
# ---------------------------

target_link_libraries(${PROJECT_NAME} PRIVATE VoltEngine)


target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Vulkan::Vulkan
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

# ---------------------------
# Windows-specific settings
# ---------------------------

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )

    target_compile_definitions(VoltEngine PRIVATE
            NOMINMAX
            WIN32_LEAN_AND_MEAN
    )
endif()

# ---------------------------
# Set startup project folder (Visual Studio)
# ---------------------------
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND
        ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/scripts/post_build.py
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Running post-build script"
)