file(
  GLOB TEST_SRCS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "*.c")

foreach(src IN LISTS TEST_SRCS)
  get_filename_component(name ${src} NAME_WE) # filename without extension

  add_executable(${name} ${src})
  add_test(NAME ${name} COMMAND ${name})

  # Either link or include .inc:
  # target_link_libraries(${name} kcli)
  target_include_directories(${name} PUBLIC ${PROJECT_SOURCE_DIR}/include)
endforeach()
