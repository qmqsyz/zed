file(GLOB example_files PRIVATE ./*.cpp)

foreach(file IN LISTS example_files)
    get_filename_component(basename ${file} NAME_WE)
    add_executable(${basename} ${file})
    target_link_libraries(${basename} zed_lib)
endforeach()