cmake_minimum_required(VERSION 3.20)

option(FORMAT_SOURCES "execute clang-format" ON)
option(WARN_EVERYTHING "enable -Weverything" OFF)
option(USE_ASAN "use address sanitizer" OFF)
option(USE_UBSAN "use undefined behavior sanitizer" OFF)

project(onpc C)
enable_testing()

if(FORMAT_SOURCES)
  file(GLOB sources LIST_DIRECTORIES false CONFIGURE_DEPENDS "*.h" "*.c")
  find_program(CLANG_FORMAT_EXE clang-format)
  add_custom_target(${PROJECT_NAME}-format ALL
    COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${sources}
  )
endif()

add_library(onpc_build_config INTERFACE)
target_include_directories(onpc_build_config
INTERFACE
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_compile_options(onpc_build_config
INTERFACE
  -Wall
  -Wextra
  -Werror
  -Wshadow
  -Werror=return-type
  -pedantic-errors
  $<$<BOOL:${WARN_EVERYTHING}>:
    -Weverything
    -Wno-declaration-after-statement
    -Wno-padded
    -Wno-switch-default
    -Wno-unsafe-buffer-usage
  >
  $<$<BOOL:${USE_ASAN}>:-fsanitize=address>
  $<$<BOOL:${USE_UBSAN}>:-fsanitize=undefined>
)
target_link_options(onpc_build_config
INTERFACE
  $<$<BOOL:${USE_ASAN}>:-fsanitize=address>
  $<$<BOOL:${USE_UBSAN}>:-fsanitize=undefined>
)

add_library(onpc onp.c)
set_property(TARGET onpc PROPERTY PUBLIC_HEADER "onp.h")
target_link_libraries(onpc onpc_build_config)
target_link_options(onpc
PRIVATE
  $<$<AND:$<STREQUAL:${CMAKE_BUILD_TYPE},Release>,$<STREQUAL:$<TARGET_PROPERTY:onpc,TYPE>,SHARED_LIBRARY>>:-s>
)
install(TARGETS onpc onpc_build_config
        EXPORT onpc-export
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/oov)
        install(EXPORT onpc-export
        FILE onpc-config.cmake
        DESTINATION share/cmake/onpc/
        EXPORT_LINK_INTERFACE_LIBRARIES)
export(EXPORT onpc-export
       FILE "${CMAKE_BINARY_DIR}/onpc-config.cmake")
add_executable(onpc_example example.c)
target_link_libraries(onpc_example onpc)
