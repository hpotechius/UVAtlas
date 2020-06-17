add_executable(UVAtlas UVAtlas.cpp)


target_link_libraries(UVAtlas PUBLIC
		      uvatlasLib
		      extraLib
		      geodesicsLib
		      isochartLib
		      OpenMeshCore
  			  OpenMeshTools)
		      #UVAtlasTool)
