add_library( isochartLib
	     barycentricparam.cpp
	     basemeshinfo.cpp
	     basemeshinfo.h
	     callbackschemer.h
	     graphcut.cpp
	     graphcut.h
	     imtcomputation.cpp
	     isochart.cpp
	     isochart.h
	     isochartconfig.h
	     isochartengine.cpp
	     isochartengine.h
	     isochartmesh.cpp
	     isochartmesh.h
	     isochartutil.cpp
	     isochartutil.h
	     isomap.cpp
	     isomap.h
	     lscmparam.cpp
	     mergecharts.cpp
	     meshapplyisomap.cpp
	     meshcommon.inl
	     meshoptimizeboundaries.cpp
	     meshoptimizestretch.cpp
	     meshpartitionchart.cpp
	     packingcharts.cpp
	     progressivemesh.cpp
	     progressivemesh.h
	     sparsematrix.hpp
	     SymmetricMatrix.hpp
	     UVAtlas.cpp
	     UVAtlasRepacker.cpp
	     UVAtlasRepacker.h
	     vertiter.cpp
	     vertiter.h
	     Vis_Maxflow.cpp
	     Vis_Maxflow.h)

target_include_directories( isochartLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries( isochartLib PUBLIC uvatlasLib
					   extraLib
					   geodesicsLib)
