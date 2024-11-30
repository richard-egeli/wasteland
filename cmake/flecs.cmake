
include(FetchContent)

FetchContent_Declare(
  flecs
  GIT_REPOSITORY https://github.com/SanderMertens/flecs
  GIT_TAG v4.0.3
)

FetchContent_MakeAvailable(flecs)

target_link_libraries(${PROJECT_NAME} flecs)
