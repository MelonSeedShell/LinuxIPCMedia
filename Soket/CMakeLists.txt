# set(CMAKE_C_COMPILER "gcc")
# set(CMAKE_CXX_COMPILER "g++")
# set(CMAKE_STRIP strip)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SOCKET)

add_library(Socket SHARED ${SOCKET})
add_library(socket ${SOCKET})

add_executable(socketclient ${CMAKE_CURRENT_SOURCE_DIR}/demo/client.cpp)
add_executable(socketserver ${CMAKE_CURRENT_SOURCE_DIR}/demo/server.cpp)

target_link_libraries(socketclient
    -Wl,--start-group
    socket
    -Wl,--end-group)

target_link_libraries(socketserver
    -Wl,--start-group
    socket
    -Wl,--end-group)