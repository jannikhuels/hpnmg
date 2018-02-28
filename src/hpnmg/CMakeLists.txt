file(GLOB_RECURSE headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
file(GLOB_RECURSE sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

add_library(${PROJECT_NAME} SHARED ${sources} ReadHybridPetrinet.cpp ReadHybridPetrinet.h datastructures/arcs/Arc.h datastructures/arcs/DiscreteArc.cpp datastructures/arcs/DiscreteArc.h datastructures/arcs/ContinuousArc.cpp datastructures/arcs/ContinuousArc.h datastructures/arcs/GuardArc.cpp datastructures/arcs/GuardArc.h datastructures/arcs/Arc.cpp datastructures/places/Place.cpp datastructures/transitions/Transition.cpp ParseHybridPetrinet.cpp ParseHybridPetrinet.h AleatoryVariable.cpp AleatoryVariable.h)
set_target_properties( ${PROJECT_NAME} PROPERTIES
	VERSION "${PROJECT_VERSION}"
	SOVERSION "${PROJECT_VERSION}"
	CLEAN_DIRECT_OUTPUT 1
	OUTPUT_NAME hpnmg
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hpnmg hypro)