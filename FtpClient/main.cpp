#include <iostream>

#include "FtpClient.h"

int main(int argc, char *argv[])
{
    // std::string ip = "211.139.163.62";
    std::string ip = "192.168.3.95";
    int port = 2121;
    std::string user = "dsjet";
    std::string pwd = "Dsj@16888";

    std::string srcPath = "./ftp/test.mp4";
    std::string dstPath = "/ftp/test/test.mp4";

    int ret = 0;
    FtpClient ftp;

    ret = ftp.init(ip, port);
    if (ret < 0) {
        std::cerr << "ftp init failed" << std::endl;
        ftp.deinit();
        return ret;
    }

    //login
    ret = ftp.login(user, pwd);
    if (ret < 0) {
        std::cerr << "ftp login failed" << std::endl;
        ftp.deinit();
        return ret;
    }

    //upload
    // ret = ftp.upload(srcPath, dstPath);
    // if (ret < 0) {
    //     std::cerr << "ftp login failed" << std::endl;
    //     ftp.deinit();
    //     return ret;
    // }

    ret = ftp.deinit();
    if (ret < 0) {
        std::cerr << "ftp deinit failed" << std::endl;
        return ret;
    }

    return 0;
}