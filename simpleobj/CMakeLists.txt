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
message(STATUS "Installing glm via COPY")
file(COPY ../external/glm DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(BUILD_STATIC_LIBS ON)
add_subdirectory(external/glm/glm)
set_property(TARGET glm_static PROPERTY FOLDER "Dependencies")

# TinyObjLoader
message(STATUS "Installing TinyObjLoader via COPY")
file(COPY ../external/tinyobjloader/tiny_obj_loader.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader)

# =============================================================

# Sources

file(GLOB_RECURSE FILE_SOURCES RELATIVE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

# Solution Filters
foreach(source IN LISTS FILE_SOURCES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    string(REPLACE "src" "" source_path_final "${source_path_msvc}")
    source_group("${source_path_final}" FILES "${source}")
endforeach()

# =============================================================

# Finalize App

add_executable(
    ${PROJECT_NAME}
    WIN32
    "${FILE_SOURCES}"
)

# =============================================================

# Finish Dependencies

target_link_libraries(
    ${PROJECT_NAME}
    glm_static
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC external/glm
    PUBLIC external/tinyobjloader
)

target_include_directories(
    glm_static
    PUBLIC external/glm
)

add_dependencies(
    ${PROJECT_NAME}
    glm_static
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
