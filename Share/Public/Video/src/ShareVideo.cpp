#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include "ShareVideo.h"


ShareVideo::ShareVideo(const unsigned long& size):m_size(size)
{
}

ShareVideo::~ShareVideo()
{
}

int ShareVideo::send(const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& frameType)
{
    if (!m_sem || !m_shm) {
        std::cerr << "no init" << std::endl;
        return -1;
    }

    if (!m_addr) {
        std::cerr << "send failed, mmap share addr is null" << std::endl;
        return -1;
    }

    while (1) {
        int ret = 0;
        ret = m_sem->waitTmOut(__MAX_CNT__);
        // ret = m_sem->wait();
        if (ret != 0) {
            std::cerr << "send failed, wait failed" << std::endl;
            return -1;
        }

        VideoShareHead* head = (VideoShareHead*)m_addr;
        // printf("send state:%d, addr:0x%x, dataLen:%d\n", head->state, head->addr, head->dataLen);
        if (head->state == 0) {//0:need write, 1:need read
            unsigned int headLen = sizeof(VideoShareHead);
            unsigned long writeDataLen = 0;
            if (len > m_size - headLen) {
                writeDataLen = m_size - headLen;
            } else {
                writeDataLen = len;
            }

            memcpy(m_addr + headLen, data, writeDataLen);
            // head->addr = m_addr + headLen;
            head->dataLen = writeDataLen;
            head->encode = encode;
            head->frameType = frameType;
            head->pts = pts;
            head->state = 1;
            ret = 0;
            break;
        } else {
            // std::cerr << "send failed, share addr need read, head->state:" + std::to_string(head->state) << std::endl;
            // printf("send failed, head->state:%d\n", head->state);
            m_sem->signal();
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
    }

    m_sem->signal();

    return 0;
}

// int ShareVideo::recv(char** data, int& len, unsigned long long& pts, int& encode, int& frameType)
int ShareVideo::recv(const VideoDataCb& videoCb)
{
    if (!m_sem || !m_shm) {
        std::cerr << "no init" << std::endl;
        return -1;
    }

    if (!m_addr) {
        std::cerr << "recv failed, mmap share addr is null" << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return -1;
    }

    while (1) {
        int ret = 0;
        ret = m_sem->waitTmOut(1);
        if (ret != 0) {
            if (__MAX_CNT__ <= m_waitRecvFailedCnt) {
                m_sem->signal();
            }
            printf("wait failed\n");
            m_waitRecvFailedCnt ++;
            return -1;
        }
        m_waitRecvFailedCnt = 0;
        VideoShareHead* head = (VideoShareHead*)m_addr;
        // printf("recv state:%d, addr:0x%x, dataLen:%d\n", head->state, head->addr, head->dataLen);
        if (head->state == 1) {//0:need write, 1:need read
            unsigned int headLen = sizeof(VideoShareHead);
            videoCb(m_addr + headLen, head->dataLen, head->pts, head->encode, head->frameType);
            head->state = 0;
            break;
        } else {
            // printf("recv failed, head->state:%d\n", head->state);
            m_sem->signal();
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
    }
    m_sem->signal();

    return 0;
}

int ShareVideo::init()
{
    FILE* file = fopen(m_path.c_str(), "r");
    if (!file) {
        file = fopen(m_path.c_str(), "w");
        if (!file) {
            std::cerr << "无法创建文件。" << std::endl;
            return -1;
        }
        m_isCreator = true;
    }
    fclose(file);
    if (m_sem) {

        return -1;
    }

    if (m_shm) {

        return -1;
    }

    int ret = 0;

    m_sem = new Sem(m_path);
    m_shm = new Shm(m_path, m_size);

    ret = m_sem->init();
    if (ret < 0) {
        std::cerr << "video init failed, sem init failed" << std::endl;
        return ret;
    }

    ret = m_shm->init();
    if (ret < 0) {
        std::cerr << "video init failed, shm init failed" << std::endl;
        return ret;
    }

    ret = m_shm->mmap((void**)&m_addr);
    if (ret < 0) {
        std::cerr << "video init failed, mmap share failed" << std::endl;
        return -1;
    }

    if (!m_addr) {
        std::cerr << "video init failed, mmap share addr is null" << std::endl;
        return -1;
    }

    if (m_addr[0] != 0 && m_addr[0] != 1) {
        memset(m_addr, 0, m_size);
    }

    return 0;
}
int ShareVideo::deinit()
{
    int ret = 0;

    ret = m_shm->unmmap((void**)&m_addr);
    if (ret < 0) {
        std::cerr << "deinit failed, unmmap share failed" << std::endl;
    }
    m_addr = nullptr;

    if (m_shm) {
        if (m_sem) {
            m_sem->waitTmOut(5);
        }
        ret = m_shm->deinit();
        if (ret < 0) {
            std::cerr << "video deinit failed, shm deinit failed" << std::endl;
            return ret;
        }

        delete m_shm;
        m_shm = nullptr;
    }

    if (m_sem) {
        // m_sem->waitTmOut(1);
        ret = m_sem->deinit();
        if (ret < 0) {
            std::cerr << "video deinit failed, sem deinit failed" << std::endl;
            return ret;
        }
        delete m_sem;
        m_sem = nullptr;
    }
printf("%s\n", __func__);
    if (!m_isCreator) {
        return 0;
    }
    remove(m_path.c_str());
    m_isCreator = false;
printf("%s:del file\n", __func__);
    return 0;
}
