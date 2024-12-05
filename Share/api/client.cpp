#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <chrono>
#include <cstdlib>
#include <string.h>
#include <vector>
#include <mutex>
#include <memory>
#include "Msg.h"
#include "ShareVideo.h"
#include "ShareTalk.h"
#include "ShareAudio.h"
#include "shareType.h"
#include "client.h"

static Msg* g_msg = nullptr;
static ShareVideo* g_shareVid = nullptr;
static ShareAudio* g_shareAud = nullptr;
static ShareTalk* g_shareTalk = nullptr;
static CLI_Ops g_cli_ops;


static int init(const CLI_Ops *ops)
{
    int ret = 0;

    memcpy(&g_cli_ops, ops, sizeof(CLI_Ops));
    g_msg = new Msg([](const MsgContent& msg){
        EVENT event;
        event.eventID = (unsigned int)msg.msgId;
        event.argv1 = msg.arg1;
        event.result = msg.arg2;
        event.createTime = msg.time;
        memcpy(event.aszPayload, msg.payload, sizeof(event.aszPayload));

        // if (event.eventID == EVENT_GET_VIDEO || event.eventID == EVENT_GET_AUDIO || event.eventID == EVENT_SND_TALK_AUDIO) {
        //     msg_inner_proc(&event);
        // } else if (g_cli_ops.cliEventCb) {
            g_cli_ops.cliEventCb(&event);
        // }
    }, true);

    ret = g_msg->init();
    if (ret < 0) {
        // std::cerr << "cli init failed , msg init failed" << std::endl;
        return -1;
    }

    g_shareVid = new ShareVideo(70 << 10);
    ret = g_shareVid->init();
    if (ret < 0) {
        // std::cerr << "cli init failed , vid init failed" << std::endl;
        return -1;
    }

    g_shareAud = new ShareAudio(2 << 10);
    ret = g_shareAud->init();
    if (ret < 0) {
        // std::cerr << "cli init failed , aud init failed" << std::endl;
        return -1;
    }

    g_shareTalk = new ShareTalk(2 << 10);
    ret = g_shareTalk->init();
    if (ret < 0) {
        // std::cerr << "cli init failed , talk init failed" << std::endl;
        return -1;
    }

    return 0;
}

static int deinit(void)
{
    int ret = 0;
    if (g_shareVid) {
        ret = g_shareVid->deinit();
        if (ret < 0) {
            // std::cerr << "cli deinit failed , vid deinit failed" << std::endl;
            return -1;
        }
        delete g_shareVid;
        g_shareVid = nullptr;
    }

    if (g_shareAud) {
        ret = g_shareAud->deinit();
        if (ret < 0) {
            // std::cerr << "cli deinit failed , aud deinit failed" << std::endl;
            return -1;
        }
        delete g_shareAud;
        g_shareAud = nullptr;
    }

    if (g_shareTalk) {
        ret = g_shareTalk->deinit();
        if (ret < 0) {
            // std::cerr << "cli deinit failed , talk deinit failed" << std::endl;
            return -1;
        }
        delete g_shareTalk;
        g_shareTalk = nullptr;
    }
// printf("cli [%s:%d]\n", __func__, __LINE__);
    if (g_msg) {
        ret = g_msg->deinit();
        if (ret < 0) {
            // std::cerr << "cli deinit failed , msg deinit failed" << std::endl;
            return -1;
        }
        delete g_msg;
        g_msg = nullptr;
    }
// printf("cli [%s:%d]\n", __func__, __LINE__);
    g_cli_ops.cliEventCb = nullptr;
    return 0;
}

static int sendAudio(char *data, int len, unsigned long long pts, int encode, int sampleRate)
{
    int ret = 0;
    if (!g_shareAud) {
        return -1;
    }

    ret = g_shareAud->send(data, len, pts, encode, sampleRate);
    return ret;
}

static int sendVideo(char *data, int len, unsigned long long pts, int encode, int frameType)
{
    int ret = 0;
    if (!g_shareVid) {
        return -1;
    }

    ret = g_shareVid->send(data, len, pts, encode, frameType);
    return ret;
}

static int getTalkAudio(void)
{
    int ret = 0;
    if (!g_shareTalk) {
        return -1;
    }

    ret = g_shareTalk->recv([](const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& sampleRate){
        if (g_cli_ops.cliTalkCb) {
            g_cli_ops.cliTalkCb(data, len, pts, encode, sampleRate);
        }
    });
    if (ret < 0) {
        // std::cerr << "cli get talk failed , talk recv failed" << std::endl;
        return -1;
    }

    return ret;
}

static int sendMsg(EVENT *event)
{
    int ret = 0;
    if (!g_msg) {
        return -1;
    }
    MsgContent msgCtt;
    msgCtt.msgId = event->eventID;
    msgCtt.arg1 = event->argv1;
    msgCtt.arg2 = event->result;
    msgCtt.time = event->createTime;
    memcpy(msgCtt.payload, event->aszPayload, sizeof(msgCtt.payload));

    ret = g_msg->send(msgCtt);
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

typedef struct {
    bool isStart = false;
    std::shared_ptr<std::thread> workProc = nullptr;
    std::mutex mtx;
} pub_task;

typedef struct {
    pub_task sendVideo;
    pub_task sendAudio;
    pub_task getTalk;
} cli_task;
static cli_task g_cli_task;

int CLI_Init(const CLI_Ops *ops)
{
    return init(ops);
}
int CLI_Deinit()
{
    return deinit();
}

int CLI_SndAudio (char *data, int len, unsigned long long pts, int encode, int sampleRate)
{
    int maxCnt = 5;
    int cnt = 0;
    int ret = 0;
    while (1) {
        ret = sendAudio(data, len, pts, encode, sampleRate);
        if (ret < 0) {
            if (maxCnt <= cnt) {
                EVENT event = {0};
                event.eventID = EVENT_GET_AUDIO;
                event.argv1 = false;
                event.result = 0;
                sendMsg(&event);
                break;
            }
            cnt ++;
            // printf("[%s]sendAudio failed cnt:%d\n", __func__, cnt);
            continue;
        }

        break;
    }
    return ret;
}
int CLI_SndVIDEO (char *data, int len, unsigned long long pts, int encode, int frameType)
{
    int maxCnt = 5;
    int cnt = 0;
    int ret = 0;
    while (1) {
        ret = sendVideo(data, len, pts, encode, frameType);
        if (ret < 0) {
            if (maxCnt <= cnt) {
                EVENT event = {0};
                event.eventID = EVENT_GET_VIDEO;
                event.argv1 = false;
                event.result = 0;
                sendMsg(&event);
                break;
            }
            cnt ++;
            // printf("[%s]sendVideo failed cnt:%d\n", __func__, cnt);
            continue;
        }

        break;
    }

    return ret;
}
int CLI_StartProcTalk(void)
{

    std::lock_guard<std::mutex> lock(g_cli_task.getTalk.mtx);
    if (g_cli_task.getTalk.isStart) {
        printf("[%s]has started\n", __func__);
        return 0;
    }

    if (g_cli_task.getTalk.workProc && g_cli_task.getTalk.workProc->joinable()) {
        printf("[%s]wait end ...\n", __func__);
        g_cli_task.getTalk.workProc->join();
        printf("[%s]wait end , and restart\n", __func__);
    }

    g_cli_task.getTalk.isStart = true;
    g_cli_task.getTalk.workProc = std::make_shared<std::thread>([](){
        int ret = 0;
        int MaxFailedCnt = 10;
        int needReset = 5;
        int failedCnt = 0;
        while (g_cli_task.getTalk.isStart) {
            ret = getTalkAudio();
            if (ret < 0) {
                if (failedCnt > MaxFailedCnt) {
                    // printf("getTalkAudio failed cnt out max, exit\n");
                    EVENT event = {0};
                    event.eventID = EVENT_SND_TALK_AUDIO;
                    event.argv1 = false;
                    event.result = 0;
                    sendMsg(&event);

                    break;
                }
                failedCnt++;
                continue;
            }
            failedCnt = 0;
        }
        g_cli_task.getTalk.isStart = false;
    });
    return 0;
}

int CLI_StopProcTalk(void)
{
    // printf("[%s:%d] \n", __func__,__LINE__);
    EVENT event;
    event.eventID = EVENT_SND_TALK_AUDIO;
    event.argv1 = false;
    event.result = 0;
    sendMsg(&event);

    g_cli_task.getTalk.isStart = false;
    if (g_cli_task.getTalk.workProc && g_cli_task.getTalk.workProc->joinable()) {
        g_cli_task.getTalk.workProc->join();
        g_cli_task.getTalk.workProc = nullptr;
    }
    // printf("[%s:%d] \n", __func__,__LINE__);
    return 0;
}

int CLI_SndMsg(EVENT *event)
{
    return sendMsg(event);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
