add_library( uvatlasTool
	     Mesh.cpp
	     Mesh.h
	     MeshOBJ.cpp
	     SDKMesh.h)

target_include_directories( uvatlasTool PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

SET_TARGET_PROPERTIES(uvatlasTool PROPERTIES LINKER_LANGUAGE C)

target_link_libraries( uvatlasTool PUBLIC uvatlasLib)
