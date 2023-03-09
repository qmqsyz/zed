aux_source_directory(util SOURCES)
aux_source_directory(coroutine SOURCES)
aux_source_directory(log SOURCES)
aux_source_directory(comm SOURCES)
aux_source_directory(net SOURCES)

add_library(zed_lib ${SOURCES})
