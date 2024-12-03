#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <cstdio>
#include <chrono>
#include "Msg.h"

Msg::Msg(const msgCb& cb):m_cb(cb)
{
}

Msg::~Msg()
{
}

int Msg::init()
{
    int ret = 0;
    std::ofstream file(m_path, std::ios::out);
    if (!file) {
        std::cerr << "无法创建文件。" << std::endl;
        return -1;
    }
    file.close();

    // 生成消息队列的键值
    m_key = ftok(m_path.c_str(), 'a');
    if (m_key == -1) {
        perror("init failed, ftok failed\n");
        return -1;
    }

    // 获取消息队列标识符msgid
    m_msgid = msgget(m_key, 0666);
    if (m_msgid == -1) {
        m_msgid = msgget(m_key, IPC_CREAT | 0666);
        if (m_msgid == -1) {
            perror("init failed, msgget failed\n");
            return -1;
        }

        m_isCreator = true;
    }

    ret = recvProc();

    return 0;
}
int Msg::deinit()
{
    int ret = 0;

    if (m_msgid == -1) {
        remove(m_path.c_str());
        return 0;
    }

    m_proc = false;

    if (m_recvThread) {
        m_recvThread->join();
    }

    if (!m_isCreator) {
        return 0;
    }
    // 标记消息队列可删除（在实际结束程序时可执行此操作）
    if (msgctl(m_msgid, IPC_RMID, NULL) == -1) {
        perror("deinit failed, msgctl failed\n");
        return -1;
    }

    m_isCreator = false;

    std::remove(m_path.c_str());

    return 0;
}

int Msg::send(const MsgContent& msg)
{
    if (m_msgid == -1) {
        std::cerr << "msg send failed, because not init " << std::endl;
        return -1;
    }

    MsgCtx msgCtx = {msg.msgId, msg};
    if (msgsnd(m_msgid, &msgCtx, sizeof(msgCtx.msgContent), 0) == -1) {
        std::cerr << "msg send failed, because msgsnd failed " << std::endl;
        return -1;
    }

    return 0;
}

int Msg::recvProc()
{
    int ret = 0;

    if (m_msgid == -1) {
        std::cerr << "not init " << std::endl;
        return -1;
    }

    if (m_recvThread) {
        return 0;
    }

    m_recvThread = new std::thread([this](){
        MsgCtx msgCtx;
        m_proc = true;
        while (m_proc) {
            if (msgrcv(m_msgid, &msgCtx, sizeof(msgCtx.msgContent), 0, IPC_NOWAIT) == -1) {
                // std::cerr << "msg recv failed" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }
            m_cb(msgCtx.msgContent);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    return 0;
}