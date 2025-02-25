#include <iostream>
#include <libusb-1.0/libusb.h>

// 定义日志输出函数，方便记录相关操作信息
static void testLog(const char* func, int line, const char* fmt,...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    _vsnprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    std::string logMessage = "[" + std::string(func) + "-" + std::to_string(line) + "]: " + buffer;
    std::cout << logMessage;
}

#define LOG(fmt, args) \
    do { \
        testLog(__func__, __LINE__, fmt, args); \
    } while (0)

// 根据指定的设备VID、PID等信息查找USB设备
libusb_device_handle* FindUsbDevice(libusb_context* ctx, uint16_t vid, uint16_t pid, int interface_number) {
    libusb_device** device_list;
    ssize_t num_devices = libusb_get_device_list(ctx, &device_list);
    if (num_devices < 0) {
        LOG("获取USB设备列表失败\n", 0);
        return nullptr;
    }

    libusb_device_handle* device_handle = nullptr;
    for (ssize_t i = 0; i < num_devices; ++i) {
        libusb_device* device = device_list[i];
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(device, &desc) == 0) {
            if (desc.idVendor == vid && desc.idProduct == pid) {
                if (libusb_open(device, &device_handle) == 0) {
                    if (libusb_claim_interface(device_handle, interface_number) == 0) {
                        break;
                    }
                    else {
                        libusb_close(device_handle);
                        device_handle = nullptr;
                    }
                }
            }
        }
    }

    libusb_free_device_list(device_list, 1);
    return device_handle;
}

// 从USB设备读取数据的函数
int ReadFromUsbDevice(libusb_device_handle* handle, char* buffer, int buffer_size) {
    int transferred;
    int result = libusb_bulk_transfer(handle, LIBUSB_ENDPOINT_IN | 0x81, (unsigned char*)buffer, buffer_size, &transferred, 0);
    if (result == 0 && transferred > 0) {
        return transferred;
    }
    return -1;
}

// 向USB设备写入数据的函数
int WriteToUsbDevice(libusb_device_handle* handle, const char* data, int data_size) {
    int transferred;
    int result = libusb_bulk_transfer(handle, LIBUSB_ENDPOINT_OUT | 0x02, (unsigned char*)data, data_size, &transferred, 0);
    if (result == 0 && transferred > 0) {
        return transferred;
    }
    return -1;
}

int main() {
    libusb_context* ctx = nullptr;
    int result = libusb_init(&ctx);
    if (result < 0) {
        LOG("初始化libusb失败\n", 0);
        return -1;
    }

    // 这里假设要查找的USB设备的VID、PID以及接口号，你需要根据实际设备进行替换
    uint16_t vid = 0x1D6B;
    uint16_t pid = 0x5228;
    int interface_number = 0;
    libusb_device_handle* hDevice = FindUsbDevice(ctx, vid, pid, interface_number);
    if (hDevice == nullptr) {
        LOG("未能找到或打开指定的USB设备\n", 0);
        libusb_exit(ctx);
        return -1;
    }

    char readBuffer[128];
    std::string input;
    while (true) {
        std::cout << "请输入要发送的数据（输入 'q' 退出）：";
        std::getline(std::cin, input);
        if (input == "q") {
            break;
        }

        int written_bytes = WriteToUsbDevice(hDevice, input.c_str(), input.size());
        if (written_bytes < 0) {
            LOG("向USB设备写入数据失败\n", 0);
        }

        int read_bytes = ReadFromUsbDevice(hDevice, readBuffer, sizeof(readBuffer));
        if (read_bytes > 0) {
            LOG("从USB设备读取的数据：%s\n", readBuffer);
        }
        else if (read_bytes == -1) {
            LOG("从USB设备读取数据失败\n", 0);
        }
    }

    libusb_release_interface(hDevice, interface_number);
    libusb_close(hDevice);
    libusb_exit(ctx);
    return 0;
}