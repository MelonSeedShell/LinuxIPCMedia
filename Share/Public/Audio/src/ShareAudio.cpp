#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include "ShareAudio.h"


ShareAudio::ShareAudio(const unsigned long& size):m_size(size)
{
}

ShareAudio::~ShareAudio()
{
}

int ShareAudio::send(const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& sampleRate)
{
    if (!m_sem || !m_shm) {
        std::cerr << "no init" << std::endl;
        return -1;
    }

    if (!m_addr) {
        // std::cerr << "send failed, mmap share addr is null" << std::endl;
        return -1;
    }
    int maxWaitReadFlg = 200;
    int waitReadCnt = 0;
    while (1) {
        int ret = 0;
        ret = m_sem->waitTmOut(__MAX_WAIT_MS__);
        if (ret != 0) {
            // std::cerr << "send failed, wait failed" << std::endl;
            return -1;
        }

        AudioShareHead* head = (AudioShareHead*)m_addr;
        // printf("send state:%d, addr:0x%x, dataLen:%d\n", head->state, head->addr, head->dataLen);
        if (head->state == 0) {//0:need write, 1:need read
            unsigned int headLen = sizeof(AudioShareHead);
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
            head->sampleRate = sampleRate;
            head->pts = pts;
            head->state = 1;
            ret = 0;
            break;
        } else {
            // std::cerr << "send failed, share addr need read, head->state:" + std::to_string(head->state) << std::endl;
            // printf("send failed, head->state:%d\n", head->state);
            m_sem->signal();
            if (waitReadCnt >= maxWaitReadFlg) {
                return -1;
            }
            waitReadCnt += 10;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
    }

    m_sem->signal();

    return 0;
}

// int ShareAudio::recv(char** data, int& len, unsigned long long& pts, int& encode, int& frameType)
int ShareAudio::recv(const AudioDataCb& videoCb)
{
    if (!m_sem || !m_shm) {
        std::cerr << "no init" << std::endl;
        return -1;
    }

    if (!m_addr) {
        // std::cerr << "recv failed, mmap share addr is null" << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return -1;
    }

    int maxWaitReadFlg = 200;
    int waitReadCnt = 0;
    while (1) {
        int ret = 0;
        ret = m_sem->waitTmOut(__MAX_WAIT_MS__);
        if (ret != 0) {
            if (__MAX_CNT__ == m_waitRecvFailedCnt && !m_semReset) {
                m_sem->signal();
                m_semReset = true;
            }
            // printf("wait failed\n");
            m_waitRecvFailedCnt ++;
            return -1;
        }
        m_waitRecvFailedCnt = 0;
        AudioShareHead* head = (AudioShareHead*)m_addr;
        // printf("recv state:%d, addr:0x%x, dataLen:%d\n", head->state, head->addr, head->dataLen);
        if (head->state == 1) {//0:need write, 1:need read
            unsigned int headLen = sizeof(AudioShareHead);
            videoCb(m_addr + headLen, head->dataLen, head->pts, head->encode, head->sampleRate);
            head->state = 0;
            waitReadCnt = 0;
            break;
        } else {
            // printf("recv failed, head->state:%d\n", head->state);
            m_sem->signal();
            if (waitReadCnt >= maxWaitReadFlg) {
                return -1;
            }
            waitReadCnt += 10;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
    }
    m_sem->signal();

    return 0;
}

int ShareAudio::init()
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
        // std::cerr << "video init failed, sem init failed" << std::endl;
        return ret;
    }

    ret = m_shm->init();
    if (ret < 0) {
        // std::cerr << "video init failed, shm init failed" << std::endl;
        return ret;
    }

    ret = m_shm->mmap((void**)&m_addr);
    if (ret < 0) {
        // std::cerr << "video init failed, mmap share failed" << std::endl;
        return -1;
    }

    if (!m_addr) {
        // std::cerr << "video init failed, mmap share addr is null" << std::endl;
        return -1;
    }

    if (m_addr[0] != 0 && m_addr[0] != 1) {
        memset(m_addr, 0, m_size);
    }

    return 0;
}
int ShareAudio::deinit()
{
    int ret = 0;

    ret = m_shm->unmmap((void**)&m_addr);
    if (ret < 0) {
        // std::cerr << "deinit failed, unmmap share failed" << std::endl;
    }
    m_addr = nullptr;

    if (m_shm) {
        if (m_sem) {
            m_sem->waitTmOut(__MAX_WAIT_MS__);
        }
        ret = m_shm->deinit();
        if (ret < 0) {
            // std::cerr << "video deinit failed, shm deinit failed" << std::endl;
            return ret;
        }

        delete m_shm;
        m_shm = nullptr;
    }

    if (m_sem) {
        ret = m_sem->deinit();
        if (ret < 0) {
            // std::cerr << "video deinit failed, sem deinit failed" << std::endl;
            return ret;
        }
        delete m_sem;
        m_sem = nullptr;
    }

    if (!m_isCreator) {
        return 0;
    }
    remove(m_path.c_str());
    m_isCreator = false;

    return 0;
}
