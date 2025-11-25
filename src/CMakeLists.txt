add_library(cstartlib cstartlib.c)

target_include_directories(cstartlib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# target_sources(cstartlib PRIVATE file.c)
