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
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
	else()
		message("-- Using clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
	endif()
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
	set(USE_CLANG PARENT SCOPE)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	message("-- Using g++")
	#if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
#		message(FATAL_ERROR " g++ version must be at least 4.9!")
	#endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
	if(NOT HYPRO_COVERAGE)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0")
	endif()
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
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
find_package(hypro CONFIG REQUIRED)
find_package(carl CONFIG REQUIRED)
find_package(XercesC REQUIRED)

find_package(Boost REQUIRED)
find_package(GSL REQUIRED)
find_package(CGAL REQUIRED)#find_package(CGAL CONFIG REQUIRED) #also link lib: sudo ln -s /usr/lib/x86_64-linux-gnu/libCGAL.so /usr/lib/x86_64-linux-gnu/libcgal.so


if ( CGAL_FOUND )

  include( ${CGAL_USE_FILE} )

  include( CGAL_CreateSingleSourceCGALProgram )

  find_package(Eigen3)
  if (EIGEN3_FOUND)
    include( ${EIGEN3_USE_FILE} )

    include_directories (BEFORE "../../include")

    #create_single_source_cgal_program( "Epick_d.cpp" )

  else()

    message(STATUS "NOTICE: This program requires the Eigen3 library, and will not be compiled.")

  endif()

else()

    message(STATUS "NOTICE: This program requires the CGAL library, and will not be compiled.")

endif()

if ( CGAL_FOUND )

  include( ${CGAL_USE_FILE} )

  include( CGAL_CreateSingleSourceCGALProgram )

  find_package(Eigen3)
  if (EIGEN3_FOUND)
    include( ${EIGEN3_USE_FILE} )

    include_directories (BEFORE "../../include")

    #create_single_source_cgal_program( "Epick_d.cpp" )

  else()

    message(STATUS "NOTICE: This program requires the Eigen3 library, and will not be compiled.")

  endif()

else()

    message(STATUS "NOTICE: This program requires the CGAL library, and will not be compiled.")

endif()


# Include files
# -----------------------------------------------------------------

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
include_directories("${PROJECT_BINARY_DIR}/src")
include_directories(${GSL_INCLUDE_DIR})

add_subdirectory(src)


# Copy files needed at runtime
# -----------------------------------------------------------------
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/test)
configure_file(${PROJECT_SOURCE_DIR}/src/hpnmg/datastructures/HPnG.xsd ${PROJECT_BINARY_DIR}/test)
configure_file(${PROJECT_SOURCE_DIR}/src/hpnmg/datastructures/HPnG.xsd ${PROJECT_BINARY_DIR})
configure_file(${PROJECT_SOURCE_DIR}/src/hpnmg/datastructures/Formula.xsd ${PROJECT_BINARY_DIR}/test)
configure_file(${PROJECT_SOURCE_DIR}/src/hpnmg/datastructures/Formula.xsd ${PROJECT_BINARY_DIR})

# Testing
# -----------------------------------------------------------------
enable_testing()
add_subdirectory(test)

# Examples
# -----------------------------------------------------------------

add_subdirectory(examples)

# Executable
# -----------------------------------------------------------------

add_executable(main main.cpp)
target_link_libraries(main hpnmg ${Boost_LIBRARIES} ${GSL_LIBRARIES})