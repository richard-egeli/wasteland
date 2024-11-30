
include(FetchContent)
FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG master
  )

FetchContent_MakeAvailable(enet)

target_include_directories(${PROJECT_NAME} PUBLIC ${enet_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} enet)
