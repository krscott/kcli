add_library(kclilib kclilib.c)

target_include_directories(kclilib PUBLIC ${PROJECT_SOURCE_DIR}/include)

install(TARGETS kclilib)
