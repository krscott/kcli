add_library(kclilib kclilib.c)

target_include_directories(kclilib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# target_sources(kclilib PRIVATE file.c)
