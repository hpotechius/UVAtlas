add_library( geodesicsLib
	     ApproximateOneToAll.cpp
	     ApproximateOneToAll.h
         datatypes.h
	     ExactOneToAll.cpp
	     ExactOneToAll.h
	     mathutils.cpp
	     mathutils.h
	     minheap.hpp)

target_include_directories( geodesicsLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries( geodesicsLib PUBLIC extraLib)
