cmake_minimum_required(VERSION 3.16)

if("$ENV{CC}" MATCHES ".*mingw32.*")
  # Required for Nix pkgCross
  message("Detected mingw32")
  set(WIN32
      ON
      CACHE BOOL "Set WIN32 for all CMake files")
  set(UNIX
      OFF
      CACHE BOOL "Unset UNIX for all CMake files")
endif()

project(c-start LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  add_compile_options(
    -Weverything
    # Based on project rules
    -Wno-declaration-after-statement
    -Wno-unsafe-buffer-usage
    -Wno-pre-c11-compat
    -Wno-covered-switch-default
    # Occasionally enable for manual review
    -Wno-padded
    -Wno-switch-default
    -Wno-used-but-marked-unused
    -Werror=sometimes-uninitialized)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

add_compile_options(-Werror=unused-result -Werror=return-type)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  message("Debug build")

  add_compile_options(-fsanitize=undefined -fsanitize=address)
  add_link_options(-fsanitize=undefined -fsanitize=address)

  if(DEFINED ENV{DISABLE_OPTIMIZATIONS})
    message("Disabled Optimization")
    add_compile_options(-O0)
  endif()
else()
  message("Release build")
  set(CMAKE_UNITY_BUILD ON)

  add_compile_options(-Os)
endif()

add_subdirectory(src)
add_subdirectory(app)

set_target_properties(cstartlib c-start PROPERTIES C_STANDARD 11)

install(TARGETS c-start)

enable_testing()
add_subdirectory(test)
