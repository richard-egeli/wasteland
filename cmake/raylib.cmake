
include(FetchContent)

FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG master
)

FetchContent_MakeAvailable(raylib)

target_link_libraries(${PROJECT_NAME} raylib)
