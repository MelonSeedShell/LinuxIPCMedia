set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_STRIP strip)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Shm/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Sem/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Msg/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Public/Video/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Public/Audio/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Public/Talk/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Shm/src Shm)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Sem/src Sem)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Msg/src Msg)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Public/Video/src VIDEO)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Public/Audio/src AUDIO)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Public/Talk/src TALK)
add_library(AVdataShare SHARED ${Sem} ${Shm} ${Msg} ${VIDEO} ${AUDIO} ${TALK})
add_library(Share ${Sem} ${Shm} ${Msg} ${VIDEO} ${AUDIO} ${TALK})

add_executable(serverShare ${CMAKE_CURRENT_SOURCE_DIR}/demo/server.cpp)

add_executable(clientShare ${CMAKE_CURRENT_SOURCE_DIR}/demo/client.cpp)

target_link_libraries(serverShare
    -Wl,--start-group
    Share
    -Wl,--end-group)

target_link_libraries(clientShare
    -Wl,--start-group
    Share
    -Wl,--end-group)