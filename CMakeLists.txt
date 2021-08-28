cmake_minimum_required(VERSION 3.14)
include(FetchContent)
project(convolution-surfaces)
set(CMAKE_CXX_STANDARD 17)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.7
)
FetchContent_MakeAvailable(glm)
include_directories(external/stb)
include_directories(${convolution-surfaces_SOURCE_DIR})
add_subdirectory(convolution-surfaces)
