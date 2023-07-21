cmake_minimum_required(VERSION 3.16)
project(mpeghdec LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 11)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Define dependencies
include(FetchContent)

# ilo from 2023-05-30
FetchContent_Declare(
  ilo
  GIT_REPOSITORY https://github.com/Fraunhofer-IIS/ilo.git
  GIT_TAG        r1.0.0
)
# mmtisobmff from 2023-05-30
FetchContent_Declare(
  mmtisobmff
  GIT_REPOSITORY https://github.com/Fraunhofer-IIS/mmtisobmff.git
  GIT_TAG        r1.0.0
)

get_directory_property(parentDir PARENT_DIRECTORY)

# Only enable building binaries by default if project is top-level
if(parentDir)
  set(mpeghdec_BUILD_BINARIES OFF CACHE BOOL   "Build demo binaries")
else()
  set(mpeghdec_BUILD_BINARIES ON  CACHE BOOL   "Build demo binaries")
endif()
set(mpeghdec_BUILD_DOC OFF CACHE BOOL "Build mpeghdec documentation")

# Add libraries
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
add_subdirectory(src)

# Add binaries
if(mpeghdec_BUILD_BINARIES)
  add_subdirectory(demo)
endif()

# Add documentation
if(mpeghdec_BUILD_DOC)
  add_subdirectory(doc)
endif()

