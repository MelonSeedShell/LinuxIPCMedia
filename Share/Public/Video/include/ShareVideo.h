#pragma once
#include <thread>
#include <functional>
#include <cstdlib>
#include "Sem.h"
#include "Shm.h"

typedef struct {
    // char* addr = nullptr;
    int state = 0; //0:writing, 1:reading
    int dataLen = 0;
    unsigned long long pts = 0;
    int encode = 0;
    int frameType = 0;
} VideoShareHead;

class ShareVideo
{
public:
    using VideoDataCb = std::function<void(const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& frameType)>;

public:
    ShareVideo(const unsigned long& size);
    ~ShareVideo();

    int init();
    int deinit();
    int send(const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& frameType);
    // int recv(char** data, int& len, unsigned long long& pts, int& encode, int& frameType);
    int recv(const VideoDataCb& videoCb);
private:
    /* data */
    // const std::string m_path = std::string(getenv("HOME")) + ".shareVideo";
    const std::string m_path = "/tmp/.shareVideo";
    bool m_isCreator = false;

    bool m_semReset = false;
    Sem* m_sem = nullptr;
    Shm* m_shm = nullptr;

    const int __MAX_WAIT_MS__ = 128;
    const int __MAX_CNT__ = 3;
    unsigned int m_waitRecvFailedCnt = 0;
    unsigned long m_size = 0;
    char* m_addr = nullptr;

    VideoDataCb m_videoCb;
};
