cmake_minimum_required(VERSION 3.20)

project(ZED LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS on)

 if(NOT DEFINED CMAKE_BUILD_TYPE)
     set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(${PROJECT_SOURCE_DIR})

# message("PROJECT_SOURCE_DIR:" ${PROJECT_SOURCE_DIR})
# message("PROJECT_BINARY_DIR:" ${PROJECT_BINARY_DIR})

add_subdirectory(zed)
add_subdirectory(tests)