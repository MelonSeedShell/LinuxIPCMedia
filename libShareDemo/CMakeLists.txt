# set(CMAKE_C_COMPILER "gcc")
# set(CMAKE_CXX_COMPILER "g++")
# set(CMAKE_STRIP strip)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_TYPE})

# add_executable(gbSvr ${CMAKE_CURRENT_SOURCE_DIR}/gbSvr.cpp)
add_executable(client ${CMAKE_CURRENT_SOURCE_DIR}/demo/client.cpp)
add_executable(server ${CMAKE_CURRENT_SOURCE_DIR}/demo/server.cpp)

# target_link_libraries(gbSvr
#     -Wl,--start-group
#     libAVdataShare.so
#     libgb28181_jni.a
#     -Wl,--end-group)

target_link_libraries(client
    -Wl,--start-group
    libAVdataShare.so
    -Wl,--end-group)

target_link_libraries(server
    -Wl,--start-group
    libAVdataShare.so
    -Wl,--end-group)
