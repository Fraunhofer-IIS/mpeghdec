cmake_minimum_required(VERSION 3.16)
project(mpeghdec
  VERSION 3.0.1
  DESCRIPTION "The Fraunhofer MPEG-H decoder"
  HOMEPAGE_URL "https://github.com/Fraunhofer-IIS/mpeghdec"
  LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

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
  GIT_TAG        r1.0.1
)

get_directory_property(parentDir PARENT_DIRECTORY)

# Set either of the two variables to OFF in case not both functionalities are required:
set(mpeghdec_BUILD_DECODER   ON  CACHE BOOL   "Build MPEG-H decoder functionality")
set(mpeghdec_BUILD_UIMANAGER ON  CACHE BOOL   "Build MPEG-H UI manager functionality")
mark_as_advanced(mpeghdec_BUILD_DECODER mpeghdec_BUILD_UIMANAGER)
if(NOT mpeghdec_BUILD_DECODER AND NOT mpeghdec_BUILD_UIMANAGER)
  message(SEND_ERROR "At least one of \"mpeghdec_BUILD_DECODER\" or \"mpeghdec_BUILD_UIMANAGER\" needs to be enabled!")
endif()


# Only enable building binaries by default if project is top-level
if(parentDir)
  set(mpeghdec_BUILD_BINARIES OFF CACHE BOOL   "Build demo binaries")
else()
  set(mpeghdec_BUILD_BINARIES ON  CACHE BOOL   "Build demo binaries")
endif()
set(mpeghdec_BUILD_DOC OFF CACHE BOOL "Build mpeghdec documentation")

# Add libraries
add_subdirectory(src)

# Add binaries
if(mpeghdec_BUILD_BINARIES)
  add_subdirectory(demo)
endif()

# Add documentation
if(mpeghdec_BUILD_DOC)
  add_subdirectory(doc)
endif()

# Install
include(GNUInstallDirs)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mpeghdec.pc.in ${CMAKE_CURRENT_BINARY_DIR}/mpeghdec.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mpeghdec.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

