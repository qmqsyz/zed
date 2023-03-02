aux_source_directory(util SOURCES)
aux_source_directory(coro SOURCES)
aux_source_directory(log SOURCES)
aux_source_directory(comm SOURCES)
# aux_source_directory(fiber SOURCES)

add_library(zed_lib ${SOURCES})

