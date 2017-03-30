# Create configure command dependend on compiler
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(cmake_command -Dgtest_force_shared_crt=ON -DCXX=/usr/bin/clang++ -DCMAKE_CXX_FLAGS=-stdlib=libc++ -DCMAKE_CXX_FLAGS=-std=c++11 -DCMAKE_CXX_FLAGS=-DGTEST_USE_OWN_TR1_TUPLE=1 )
else()
		set(cmake_command -Dgtest_force_shared_crt=ON)
endif()
# Add gtest (local build)
ExternalProject_Add(
	googletest
	GIT_REPOSITORY http://github.com/google/googletest.git
	# Force separate output paths for debug and release builds to allow easy
	# identification of correct lib in subsequent TARGET_LINK_LIBRARIES
	#CMAKE_ARGS ${cmake_command}
	GIT_TAG "release-1.7.0"
	INSTALL_COMMAND "")

# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
ExternalProject_Get_Property(googletest binary_dir)

# required for testing.
set( GTEST_INCLUDE_DIRS "${source_dir}/include" PARENT_SCOPE)
set( GTEST_LIBRARIES "${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}" "${binary_dir}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}" pthread dl PARENT_SCOPE)

