cmake_minimum_required(VERSION 3.5)
project(UVAtlas VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -std=gnu++11")



#find_package( OpenMesh )

#add_subdirectory("lib/glmLib")
add_subdirectory("lib/glm")
add_subdirectory("lib/uvatlasLib")
add_subdirectory("lib/extraLib")
add_subdirectory("lib/geodesicsLib")
add_subdirectory("lib/isochartLib")
#add_subdirectory("lib/UVAtlasTool")
#add_subdirectory("sample")
