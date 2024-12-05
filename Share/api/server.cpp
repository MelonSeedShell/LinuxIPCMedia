#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <chrono>
#include <cstdlib>
#include <string.h>
#include <mutex>
#include <memory>
#include "Msg.h"
#include "ShareVideo.h"
#include "ShareTalk.h"
#include "ShareAudio.h"
#include "shareType.h"
#include "server.h"

static Msg* g_msg = nullptr;
static ShareVideo* g_shareVid = nullptr;
static ShareAudio* g_shareAud = nullptr;
static ShareTalk* g_shareTalk = nullptr;
static SVR_Ops g_svr_ops;

static int init(const SVR_Ops *ops)
{
    int ret = 0;
    if (!ops) {
        return -1;
    }
    memcpy(&g_svr_ops, ops, sizeof(SVR_Ops));

    g_msg = new Msg([](const MsgContent& msg){
        EVENT event;
        event.eventID = (unsigned int)msg.msgId;
        event.argv1 = msg.arg1;
        event.result = msg.arg2;
        event.createTime = msg.time;
        memcpy(event.aszPayload, msg.payload, sizeof(event.aszPayload));
        if (g_svr_ops.svrEventCb) {
            g_svr_ops.svrEventCb(&event);
        }
    }, false);

    ret = g_msg->init();
    if (ret < 0) {
        // std::cerr << "svr init failed , msg init failed" << std::endl;
        return -1;
    }

    g_shareVid = new ShareVideo(70 << 10);
    ret = g_shareVid->init();
    if (ret < 0) {
        // std::cerr << "svr init failed , vid init failed" << std::endl;
        return -1;
    }

    g_shareAud = new ShareAudio(2 << 10);
    ret = g_shareAud->init();
    if (ret < 0) {
        // std::cerr << "svr init failed , aud init failed" << std::endl;
        return -1;
    }

    g_shareTalk = new ShareTalk(2 << 10);
    ret = g_shareTalk->init();
    if (ret < 0) {
        // std::cerr << "svr init failed , talk init failed" << std::endl;
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
            // std::cerr << "svr deinit failed , vid deinit failed" << std::endl;
            return -1;
        }
        delete g_shareVid;
        g_shareVid = nullptr;
    }

    if (g_shareAud) {
        ret = g_shareAud->deinit();
        if (ret < 0) {
            // std::cerr << "svr deinit failed , aud deinit failed" << std::endl;
            return -1;
        }
        delete g_shareAud;
        g_shareAud = nullptr;
    }

    if (g_shareTalk) {
        ret = g_shareTalk->deinit();
        if (ret < 0) {
            // std::cerr << "svr deinit failed , talk deinit failed" << std::endl;
            return -1;
        }
        delete g_shareTalk;
        g_shareTalk = nullptr;
    }

    if (g_msg) {
        ret = g_msg->deinit();
        if (ret < 0) {
            // std::cerr << "cli deinit failed , msg deinit failed" << std::endl;
            return -1;
        }
        delete g_msg;
        g_msg = nullptr;
    }

    memset(&g_svr_ops, 0, sizeof(SVR_Ops));
    return 0;
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

static int getAudio(void)
{
    int ret = 0;
    if (!g_shareAud) {
        return -1;
    }

    ret = g_shareAud->recv([](const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& sampleRate){
        if (g_svr_ops.svrAudioCb) {
            g_svr_ops.svrAudioCb(data, len, pts, encode, sampleRate);
        }
    });

    if (ret < 0) {
        // std::cerr << "svr get audio failed , aud recv failed" << std::endl;
        return -1;
    }

    return ret;
}

static int getVideo(void)
{
    int ret = 0;
    if (!g_shareVid) {
        return -1;
    }

    ret = g_shareVid->recv([](const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& frameType){
        if (g_svr_ops.svrVideoCb) {
            g_svr_ops.svrVideoCb(data, len, pts, encode, frameType);
        }
    });

    if (ret < 0) {
        // std::cerr << "svr get video failed , vid recv failed" << std::endl;
        return -1;
    }

    return ret;
}

static int sendTalkAudio(const char *data, int len, unsigned long long pts, int encode, int sampleRate)
{
    int ret = 0;
    if (!g_shareTalk) {
        return -1;
    }

    ret = g_shareTalk->send(data, len, pts, encode, sampleRate);
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
    pub_task getVideo;
    pub_task getAudio;
    pub_task sendTalk;
} svr_task;
static svr_task g_svr_task;

int SVR_Init(const SVR_Ops *ops)
{
    return init(ops);
}
int SVR_Deinit(void)
{
    return deinit();
}
int SVR_StartGetAudio(void)
{
    std::lock_guard<std::mutex> lock(g_svr_task.getAudio.mtx);
    EVENT event;
    event.eventID = EVENT_GET_AUDIO;
    event.argv1 = true;
    event.result = -1;
    sendMsg(&event);

    if (g_svr_task.getAudio.isStart) {
        printf("[%s]has started\n", __func__);
        return 0;
    }

    if (g_svr_task.getAudio.workProc && g_svr_task.getAudio.workProc->joinable()) {
        printf("[%s]wait end ...\n", __func__);
        g_svr_task.getAudio.workProc->join();
        printf("[%s]wait end , and restart\n", __func__);
    }

    g_svr_task.getAudio.isStart = true;
    g_svr_task.getAudio.workProc = std::make_shared<std::thread>([](){
        int ret = 0;
        int MaxFailedCnt = 10;
        int needReset = 5;
        int failedCnt = 0;
        while (g_svr_task.getAudio.isStart) {
            ret = getAudio();
            if (ret < 0) {
                if (failedCnt > MaxFailedCnt) {
                    // printf("getAudio failed cnt out max, exit\n");
                    break;
                }
                
                if (failedCnt == needReset) {
                    EVENT event;
                    event.eventID = EVENT_GET_AUDIO;
                    event.argv1 = true;
                    event.result = -1;
                    sendMsg(&event);
                }
                failedCnt++;
                // printf("getAudio failed failedCnt:%d \n", failedCnt);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            failedCnt = 0;
        }
        g_svr_task.getAudio.isStart = false;
    });

    return 0;
}
int SVR_StopGetAudio(void)
{
    EVENT event;
    event.eventID = EVENT_GET_AUDIO;
    event.argv1 = false;
    event.result = -1;
    sendMsg(&event);

    g_svr_task.getAudio.isStart = false;
    if (g_svr_task.getAudio.workProc && g_svr_task.getAudio.workProc->joinable()) {
        g_svr_task.getAudio.workProc->join();
        g_svr_task.getAudio.workProc = nullptr;
    }
    return 0;
}
int SVR_StartGetVideo(void)
{
    std::lock_guard<std::mutex> lock(g_svr_task.getVideo.mtx);
    EVENT event;
    event.eventID = EVENT_GET_VIDEO;
    event.argv1 = true;
    event.result = -1;
    sendMsg(&event);

    if (g_svr_task.getVideo.isStart) {
        printf("[%s]has started\n", __func__);
        return 0;
    }

    if (g_svr_task.getVideo.workProc && g_svr_task.getVideo.workProc->joinable()) {
        printf("[%s]wait end ...\n", __func__);
        g_svr_task.getVideo.workProc->join();
        printf("[%s]wait end , and restart\n", __func__);
    }

    g_svr_task.getVideo.isStart = true;
    g_svr_task.getVideo.workProc = std::make_shared<std::thread>([](){
        int ret = 0;
        int MaxFailedCnt = 10;
        int needReset = 5;
        int failedCnt = 0;
        while (g_svr_task.getVideo.isStart) {
            char* data = NULL;
            int len = 0;
            unsigned long long pts = 0;
            int encode = 0;
            int frameType = 0;
            // ret = getVideo(&data, len, pts, encode, frameType);
            ret = getVideo();
            if (ret < 0) {
                if (failedCnt > MaxFailedCnt) {
                    printf("getVideo failed cnt out max, exit\n");
                    break;
                }
                
                if (failedCnt == needReset) {
                    EVENT event;
                    event.eventID = EVENT_GET_VIDEO;
                    event.argv1 = true;
                    event.result = -1;
                    sendMsg(&event);
                }
                failedCnt++;
                printf("getVideo failed failedCnt:%d \n", failedCnt);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            failedCnt = 0;
        }
        g_svr_task.getVideo.isStart = false;
    });

    return 0;
}
int SVR_StopGetVideo(void)
{
    // printf("[%s:%d] \n", __func__,__LINE__);
    EVENT event;
    event.eventID = EVENT_GET_VIDEO;
    event.argv1 = false;
    event.result = -1;
    sendMsg(&event);

    g_svr_task.getVideo.isStart = false;
    if (g_svr_task.getVideo.workProc && g_svr_task.getVideo.workProc->joinable()) {
        g_svr_task.getVideo.workProc->join();
        g_svr_task.getVideo.workProc = nullptr;
    }
    // printf("[%s:%d] \n", __func__,__LINE__);
    return 0;
}

int SVR_RequestTalk(bool bTalk)
{
    EVENT event;
    event.eventID = EVENT_SND_TALK_AUDIO;
    event.argv1 = bTalk;
    event.result = -1;
    return sendMsg(&event);
}
int SVR_SndTalkAudio (const char *data, int len, unsigned long long pts, int encode, int sampleRate)
{
    int ret = 0;
    ret = sendTalkAudio(data, len, pts, encode, sampleRate);
    if (ret < 0) {
        EVENT event;
        event.eventID = EVENT_SND_TALK_AUDIO;
        event.argv1 = true;
        event.result = -1;
        sendMsg(&event);
    }
    return ret;
}

int SVR_Login(bool bLogin)
{
    EVENT event;
    event.eventID = EVENT_LOGIN_STATUS;
    event.argv1 = bLogin;
    event.result = -1;
    return sendMsg(&event);
}
int SVR_RequestFileDir(void)
{
    EVENT event;
    event.eventID = EVENT_CTL_GET_DIR;
    return sendMsg(&event); 
}
int SVR_RequestGetDevInfo(void)
{
    EVENT event;
    event.eventID = EVENT_CTL_GET_DEV_INFO;
    return sendMsg(&event);
}
int SVR_RequestStartRec(void)
{
    EVENT event;
    event.eventID = EVENT_CTL_START_REC;
    event.argv1 = true;
    event.result = -1;
    return sendMsg(&event);
}
int SVR_RequestStopRec(void)
{
    EVENT event;
    event.eventID = EVENT_CTL_START_REC;
    event.argv1 = false;
    event.result = -1;
    return sendMsg(&event);
}
int SVR_RequestTakePhoto(void)
{
    EVENT event;
    event.eventID = EVENT_CTL_TAKE_PHOTO;
    event.result = -1;
    return sendMsg(&event);
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */
