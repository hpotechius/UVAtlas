add_library( extraLib
	     maxheap.hpp
	     pch.h)

target_include_directories( extraLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
SET_TARGET_PROPERTIES(extraLib PROPERTIES LINKER_LANGUAGE C)

target_link_libraries( extraLib PUBLIC uvatlasLib)
