cmake_minimum_required(VERSION 3.20)

project(ZED LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

if(DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "Setting default CMAKE_BUILD_TYPE to Release")
endif()

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(zed)
add_subdirectory(test)

set(LIBS
    yaml-cpp
    pthread
)

target_link_libraries(zed_lib ${LIBS})