cmake_minimum_required(VERSION 3.26)

project(ZED LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# if(NOT DEFINED CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug)

# endif()
message(STATUS ${CMAKE_BUILD_TYPE})

message(${CMAKE_SOURCE_DIR})

set(LIBS
    yaml-cpp
    pthread
)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(${PROJECT_SOURCE_DIR}/zed)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)

target_link_libraries(zed_lib ${LIBS})