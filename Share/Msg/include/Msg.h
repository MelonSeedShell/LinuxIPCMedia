#pragma once
#include <string>
#include <thread>
#include <cstdlib>
#include <functional>

typedef struct {
    int msgId;
    int arg1;
    int arg2;
    long time;
    char payload[512];
} MsgContent;

typedef struct {
    long msgId;
    MsgContent msgContent;
} MsgCtx;

class Msg
{
public:
    using msgCb = std::function<void(const MsgContent& msg)>;
public:
    Msg(const msgCb& cb, const bool& isClient);
    ~Msg();

    int init();
    int deinit();

    int send(const MsgContent& msg);

private:
    int recvProc();

private:
    // const std::string m_path = std::string(getenv("HOME")) + ".msg";
    const std::string m_path = "/tmp/.msg";
    msgCb m_cb;

    bool m_proc = false;
    std::thread* m_recvThread = nullptr;

    bool m_isClient;

    bool m_isCreator = false;
    int m_key = -1;
    int m_msgid = -1;
};

