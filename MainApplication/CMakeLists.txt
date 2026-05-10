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

add_library(VoltEngine ${VOLTENGINE_SOURCE})

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

target_include_directories(VoltEngine
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibraries/glm
)

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

target_link_libraries(${PROJECT_NAME} PRIVATE VoltEngine)

# ---------------------------
# Link Libraries
# ---------------------------

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Vulkan::Vulkan
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

target_link_libraries(VoltEngine
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
