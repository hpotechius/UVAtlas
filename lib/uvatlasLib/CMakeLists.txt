add_library( uvatlasLib
	     sal.h
	     dxgiformat.h
	     UVAtlas.h)

target_include_directories( uvatlasLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

SET_TARGET_PROPERTIES(uvatlasLib PROPERTIES LINKER_LANGUAGE C)

#target_link_libraries( uvatlasLib PUBLIC glmLib)
target_link_libraries( uvatlasLib PUBLIC glm)