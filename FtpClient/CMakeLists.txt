

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_STRIP strip)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src FTP)

add_executable(dsjFtp ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp ${FTP})

