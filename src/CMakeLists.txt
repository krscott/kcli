add_library(kcli kcli.c)

target_include_directories(kcli PUBLIC ${PROJECT_SOURCE_DIR}/include)

install(TARGETS kcli)
