# Build examples
# -----------------------------------------------------------------

file(GLOB EXAMPLES_SRC_FILES ${PROJECT_SOURCE_DIR}/examples/*.cpp)

foreach(EXAMPLE_FILE ${EXAMPLES_SRC_FILES})

	string(REPLACE ".cpp" "" EXAMPLE_NAME_FULL_PATH ${EXAMPLE_FILE})
	string(REPLACE "${PROJECT_SOURCE_DIR}/examples/" "" EXAMPLE_NAME ${EXAMPLE_NAME_FULL_PATH})

	add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})

	target_link_libraries(${EXAMPLE_NAME}
			${PROJECT_NAME}
			${XercesC_LIBRARY}
			)

endforeach()
