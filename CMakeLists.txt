cmake_minimum_required(VERSION 3.26)

project(ZED LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS on)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

if(NOT DEFINED CMAKE_BUILD_TYPE)
     set(CMAKE_BUILD_TYPE Release)
endif()

find_package(yaml )



include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(zed)
add_subdirectory(tests)

set(LIBS 
yaml-cpp
pthread
)

target_link_libraries(zed_lib ${LIBS})