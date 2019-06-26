include_directories(${GTEST_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src)

# Copy Model files for unit Tests
# -----------------------------------------------------------------
file(GLOB MODEL_TEST_FILES ${PROJECT_SOURCE_DIR}/test/testfiles/*.xml)

foreach(MODEL_FILE ${MODEL_TEST_FILES})
	configure_file(${MODEL_FILE} ${PROJECT_BINARY_DIR}/test)
endforeach()

# Add Unit Tests
# -----------------------------------------------------------------

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

foreach(TEST_FILE ${TEST_SRC_FILES})

	string(REPLACE ".cpp" "" TEST_NAME_FULL_PATH ${TEST_FILE})
	string(REPLACE "${PROJECT_SOURCE_DIR}/test/" "" TEST_NAME ${TEST_NAME_FULL_PATH})

	add_executable(${TEST_NAME} ${TEST_FILE})

	add_dependencies(${TEST_NAME} googletest)

	target_link_libraries(${TEST_NAME}
        	${PROJECT_NAME}
			${GTEST_LIBRARIES}
			${XercesC_LIBRARIES}
		)

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
