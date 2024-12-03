#pragma once
#include <thread>
#include <functional>
#include "Sem.h"
#include "Shm.h"

typedef struct {
    int state = 0; //0:writing, 1:reading
    int dataLen = 0;
    unsigned long long pts = 0;
    int encode = 0;
    int sampleRate = 0;
} TalkShareHead;

class ShareTalk
{
public:
    using TalkDataCb = std::function<void(const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& sampleRate)>;

public:
    ShareTalk(const unsigned long& size);
    ~ShareTalk();

    int init();
    int deinit();
    int send(const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& sampleRate);
    int recv(const TalkDataCb& videoCb);
private:
    /* data */
    const std::string m_path = ".ShareTalk";
    bool m_isCreator = false;

    Sem* m_sem = nullptr;
    Shm* m_shm = nullptr;

    const int __MAX_CNT__ = 3;
    int m_waitRecvFailedCnt = 0;
    unsigned long m_size = 0;
    char* m_addr = nullptr;

    TalkDataCb m_videoCb;
};
