set(CMAKE_C_COMPILER "x86_64-w64-mingw32-gcc")
set(CMAKE_CXX_COMPILER "x86_64-w64-mingw32-g++")
set(CMAKE_STRIP x86_64-w64-mingw32-strip)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_TYPE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libusb)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libusb/libusb)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libusb/libusb/os)


add_executable(usb.txt ${CMAKE_CURRENT_SOURCE_DIR}/usb.cpp)



