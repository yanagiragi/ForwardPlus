# Project Info

cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
cmake_policy(VERSION 3.6)
project("template"
        VERSION 1.0.0.0
        LANGUAGES C CXX
)

# =============================================================

# CMake Settings

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_SUPPRESS_REGENERATION true)
set(DCMAKE_GENERATOR_PLATFORM "x64")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

# =============================================================

# Options

# =============================================================

# Dependencies

# GLM
# message(STATUS "Installing glm via COPY")
# file(COPY ../external/glm DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/external)
# set(BUILD_STATIC_LIBS ON)
# add_subdirectory(external/glm/glm)
# set_property(TARGET glm_static PROPERTY FOLDER "Dependencies")

# TinyObjLoader
message(STATUS "Installing TinyObjLoader")
file(COPY ../external/tinyobjloader/tiny_obj_loader.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader)

# DirectXTK
message(STATUS "Installing DirectXTK")
file(COPY ../external/DirectXTK DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(BUILD_TOOLS OFF)
set(BUILD_XAUDIO_WIN10 OFF)
set(BUILD_XAUDIO_WIN8 OFF)
set(BUILD_XAUDIO_WIN7 OFF)
add_subdirectory(external/DirectXTK)
set_property(TARGET DirectXTK PROPERTY FOLDER "Dependencies")

message(STATUS "Installing Dear Imgui")
file(GLOB IMGUI_FILES
  "../external/imgui/*.h"
  "../external/imgui/*.cpp"
  "../external/imgui/backends/imgui_impl_win32*"
  "../external/imgui/backends/imgui_impl_dx11*"
)
file(COPY ${IMGUI_FILES} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

message(STATUS "Installing DirectXTK-DebugDraw")
file(COPY ../external/DirectXTK-DebugDraw DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/external)

# =============================================================

# Sources

file(GLOB_RECURSE FILE_SOURCES RELATIVE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

file(GLOB_RECURSE DEBUG_DRAW_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/external/DirectXTK-DebugDraw/*.cpp
)

file(GLOB_RECURSE IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/*.cpp
)

# Solution Filters
foreach(source IN LISTS FILE_SOURCES)
    source_group("" FILES "${source}")
endforeach()

foreach(source IN LISTS IMGUI_SOURCES)
    source_group("imgui" FILES "${source}")
endforeach()

foreach(source IN LISTS DEBUG_DRAW_SOURCES)
    source_group("DebugDraw" FILES "${source}")
endforeach()

# =============================================================

# Finalize App

add_executable(
    ${PROJECT_NAME}
    WIN32
    "${FILE_SOURCES}"
    "${IMGUI_SOURCES}"
    "${DEBUG_DRAW_SOURCES}"
)

# =============================================================

# Finish Dependencies

target_link_libraries(
    ${PROJECT_NAME}
    DirectXTK
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC external/tinyobjloader
    PUBLIC external/DirectXTK/Inc
    PUBLIC external/DirectXTK/Src # for pch.h
    PUBLIC external/DirectXTK-DebugDraw
    PUBLIC external/Imgui
)

add_dependencies(
    ${PROJECT_NAME}
    DirectXTK
)

# =============================================================

# Finish Settings

# Change output dir to bin
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
# Change working directory to top dir to access `assets/shaders/` folder
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/..)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
