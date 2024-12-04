if(NOT DEFINED ENV{EMSCRIPTEN_ROOT})
  message(FATAL_ERROR "EMSCRIPTEN_ROOT environment variable not set")
endif()

if(NOT DEFINED ENV{EMSCRIPTEN_EMULATOR})
  message(FATAL_ERROR "EMSCRIPTEN_EMULATOR environment variable not set")
endif()

# # This line is used to set your executable to build with the emscripten html #
# template so that you can directly open it. set(CMAKE_EXECUTABLE_SUFFIX
# ".html")

# Inside libexec
include($ENV{EMSCRIPTEN_ROOT}/cmake/Modules/Platform/Emscripten.cmake)

# Usually NodeJS or something like it
set(CMAKE_CROSSCOMPILING_EMULATOR $ENV{EMSCRIPTEN_EMULATOR})

set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} \
    -s USE_GLFW=3 \
    -s ASSERTIONS=1 \
    -s WASM=1 \
    -s ASYNCIFY \
    -s GL_ENABLE_GET_PROC_ADDRESS=1 \
    -s STACK_SIZE=1mb \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORTED_RUNTIME_METHODS=cwrap \
    --preload-file ../resources")

add_compile_definitions(PLATFORM_WEB on)
