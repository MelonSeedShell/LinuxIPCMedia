cmake_minimum_required(VERSION 3.10)
project(broadcast)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加库文件
add_library(broadcast_lib
    src/BroadcastServer.cpp
    src/BroadcastClient.cpp
)

# 添加可执行文件
add_executable(broadcast_server src/server_main.cpp)
add_executable(broadcast_client src/client_main.cpp)

# 链接库
target_link_libraries(broadcast_server broadcast_lib)
target_link_libraries(broadcast_client broadcast_lib) 