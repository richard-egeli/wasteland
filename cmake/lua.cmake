
include(FetchContent)
FetchContent_Declare(
  Lua
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://lua.org/ftp/lua-5.4.7.tar.gz
)

FetchContent_MakeAvailable(lua)

set(LUAJIT_LIBRARIES /usr/local/lib/libluajit-5.1.a)
set(LUAJIT_INCLUDE_DIR /usr/local/include/luajit-2.1)

target_include_directories(${PROJECT_NAME} PUBLIC ${LUAJIT_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${LUAJIT_LIBRARIES})
