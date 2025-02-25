# set(CMAKE_C_COMPILER "gcc")
# set(CMAKE_CXX_COMPILER "g++")
# set(CMAKE_STRIP strip)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORM_TYPE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libusb)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libusb/libusb)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libusb/libusb/os)


add_executable(testUsb ${CMAKE_CURRENT_SOURCE_DIR}/usb.cpp)

# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/mingw32/usb.exe DESTINATION "${CMAKE_BINARY_DIR}")

