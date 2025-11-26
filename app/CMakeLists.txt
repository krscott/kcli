add_executable(kcli-example main.c)

target_link_libraries(kcli-example kcli)

install(TARGETS kcli-example)
