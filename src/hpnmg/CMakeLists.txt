file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
file(GLOB_RECURSE sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB_RECURSE csources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)

add_library(${PROJECT_NAME} SHARED ${sources} ${csources})
set_target_properties( ${PROJECT_NAME} PROPERTIES
	VERSION "${PROJECT_VERSION}"
	SOVERSION "${PROJECT_VERSION}"
	CLEAN_DIRECT_OUTPUT 1
	OUTPUT_NAME hpnmg
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(hpnmg hypro-shared)
target_link_libraries(hpnmg ${hypro_LIBRARIES})
target_link_libraries(hpnmg ${XercesC_LIBRARIES})
target_link_libraries(hpnmg ${CGAL_LIBRARIES})
target_link_libraries(hpnmg ${GSL_LIBRARIES})


#message(STATUS "dyld libary path: ${DYLD_LIBRARY_PATH}")
#message(STATUS "ld libary path: ${LD_LIBRARY_PATH}")
#message(STATUS "dyld fallback libary path: ${DYLD_FALLBACK_LIBRARY_PATH}")

# experimental
#message(STATUS "Linking to hypro libraries: ${hypro_LIBRARIES}")
#message(STATUS "Hypro include dir: ${hypro_INCLUDE_DIR}")
#message(STATUS "Hypro include dirs: ${hypro_INCLUDE_DIRS}")

#include_directories(${hypro_INCLUDE_DIR})
#include_directories(${hypro_INCLUDE_DIRS})
