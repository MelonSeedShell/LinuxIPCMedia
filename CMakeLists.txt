cmake_minimum_required(VERSION 3.10)
project(AVSHARE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/${MEDIA_TYPE}/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/${MEDIA_TYPE}/src SRC)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/lib)
add_executable(avShareSvr ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/src/main.cpp ${SRC})

target_link_libraries(avShareSvr
    -Wl,--start-group
    libAVdataShare.so
    libgb28181_jni.so
    -Wl,--end-group)