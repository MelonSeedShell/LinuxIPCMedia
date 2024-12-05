
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${MEDIA_TYPE}/include)


add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp )

target_link_libraries(${TARGET_NAME}
    -Wl,--start-group
    libAVdataShare.so
    libgb28181_jni.a
    -Wl,--end-group)