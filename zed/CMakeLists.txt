aux_source_directory(log SOURCES)
aux_source_directory(. SOURCES)

add_library(zed_lib ${SOURCES})

target_link_libraries(zed_lib pthread)