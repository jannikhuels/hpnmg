cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
project(hpnmg)

# Project Properties
# -----------------------------------------------------------------

set(PROJECT_FULLNAME "HPnmG")
set(PROJECT_DESCRIPTION "A Library for the Analysis of Hybrid Petri nets with multiple general transitions.")
set(hpnmg_MAJORVERSION 0)
set(hpnmg_MINORVERSION 1)
set(hpnmg_MAINTENANCEVERSION 0)
set(hpnmg_VERSION "${hpnmg_MAJORVERSION}.${hpnmg_MINORVERSION}.${hpnmg_MAINTENANCEVERSION}")
set(PROJECT_VERSION "${hpnmg_VERSION}")
set(hpnmg_NAME "hpnmg" )
set(hpnmg_DESCRIPTION ${PROJECT_DESCRIPTION})

# Configuration Header File
# -----------------------------------------------------------------

configure_file (
  "${PROJECT_SOURCE_DIR}/src/HpnmgConfig.h.in"
  "${PROJECT_BINARY_DIR}/src/HpnmgConfig.h"
  )

# Export settings
# -----------------------------------------------------------------

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	if (APPLE)
		message("-- Using clang on apple")
		set(CMAKE_MACOSX_RPATH 1)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
	else()
		message("-- Using clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
	endif()
	set(CMAKE_CXX_RELEASE_FLAGS "${CMAKE_CXX_RELEASE_FLAGS} -O3")
	set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -g -O0")
	set(USE_CLANG PARENT SCOPE)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	message("-- Using g++")
	#if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
#		message(FATAL_ERROR " g++ version must be at least 4.9!")
	#endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
	if(NOT HYPRO_COVERAGE)
		set(CMAKE_CXX_RELEASE_FLAGS "${CMAKE_CXX_RELEASE_FLAGS} -O3")
	else()
		set(CMAKE_CXX_RELEASE_FLAGS "${CMAKE_CXX_RELEASE_FLAGS} -O0")
	endif()
	set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -g -O1")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto")
	endif()
else()
	message("-- Possibly unsupported compiler")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

# External Libraries
# -----------------------------------------------------------------

add_subdirectory(resources)
find_package(hypro REQUIRED)
find_package(carl REQUIRED)
find_package(XercesC REQUIRED)


# Include files
# -----------------------------------------------------------------

include_directories("${PROJECT_BINARY_DIR}/src")
add_subdirectory(src)

# Copy files needed at runtime
# -----------------------------------------------------------------
configure_file(${PROJECT_SOURCE_DIR}/src/hpnmg/datastructures/HPnG.xsd ${PROJECT_BINARY_DIR}/test)
configure_file(${PROJECT_SOURCE_DIR}/src/hpnmg/datastructures/HPnG.xsd ${PROJECT_BINARY_DIR})

# Testing
# -----------------------------------------------------------------

# Examples
# -----------------------------------------------------------------

add_subdirectory(examples)

enable_testing()
add_subdirectory(test)

# Executable
# -----------------------------------------------------------------

add_executable(main main.cpp)
target_link_libraries(main hpnmg)