
include(FetchContent)

FetchContent_Declare(
  box2d 
  GIT_REPOSITORY https://github.com/erincatto/box2d.git
  GIT_TAG v3.0.0
)

FetchContent_MakeAvailable(box2d)
target_link_libraries(${PROJECT_NAME} box2d)
