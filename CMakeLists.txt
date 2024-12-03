cmake_minimum_required(VERSION 3.10)
project(AVSHARE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/FtpClient)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Share)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/${MEDIA_TYPE}/include)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/lib)
add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_TYPE}/src/main.cpp )

target_link_libraries(${TARGET_NAME}
    -Wl,--start-group
    libAVdataShare.so
    libgb28181_jni.a
    -Wl,--end-group)