#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <chrono>
#include <cstdlib>
#include <string.h>
#include <mutex>
#include "Msg.h"
#include "ShareVideo.h"
#include "shareType.h"



typedef int (*GetAudioCb) (const char *data, int len, unsigned long long pts, int encode, int sampleRate);
typedef int (*GetVideoCb) (const char *data, int len, unsigned long long pts, int frameType, int encode);

typedef struct {
    EVTHUB_EVENTPROC_FN_PTR svrEventCb;
    GetAudioCb              svrAudioCb;
    GetVideoCb              svrVideoCb;
} SVR_Ops;

static Msg* g_msg = nullptr;
static ShareVideo* g_shareVid = nullptr;
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
    });

    ret = g_msg->init();
    if (ret < 0) {
        std::cerr << "svr init failed , msg init failed" << std::endl;
        return -1;
    }

    g_shareVid = new ShareVideo(70 << 10);
    ret = g_shareVid->init();
    if (ret < 0) {
        std::cerr << "svr init failed , vid init failed" << std::endl;
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
            std::cerr << "cli deinit failed , vid deinit failed" << std::endl;
            return -1;
        }
        delete g_shareVid;
        g_shareVid = nullptr;
    }

    if (g_msg) {
        ret = g_msg->deinit();
        if (ret < 0) {
            std::cerr << "cli deinit failed , msg deinit failed" << std::endl;
            return -1;
        }
        delete g_msg;
        g_msg = nullptr;
    }

    memset(&g_svr_ops, 0, sizeof(SVR_Ops));
    return 0;
}

static int getAudio(void)
{

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

// static int getVideo(char** data, int& len, unsigned long long& pts, int& encode, int& frameType)
static int getVideo(void)
{
    int ret = 0;
    if (!g_shareVid) {
        return -1;
    }
    // ret = g_shareVid->startRecv(data, len, pts, encode, frameType);
    ret = g_shareVid->startRecv([](const char* data, const int& len, const unsigned long long& pts, const int& encode, const int& frameType){
        if (g_svr_ops.svrVideoCb) {
            g_svr_ops.svrVideoCb(data, len, pts, encode, frameType);
        }
    });
    if (ret < 0) {
        std::cerr << "svr get video failed , vid recv failed" << std::endl;
        return -1;
    }

    return ret;
}


static int sendTalkAudio(char *data, int len, unsigned long long *pts, int *encode, int *sampleRate)
{

    return 0;
}




#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

typedef struct {
    bool isStart = false;
    std::thread* workProc = NULL;
    std::mutex mtx;
} demo_task;

typedef struct {
    demo_task getVideo;
    demo_task getAudio;
    demo_task sendTalk;
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
    return getAudio();
}
int SVR_StopGetAudio(void)
{
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
        printf("has started\n");
        return 0;
    }

    if (g_svr_task.getVideo.workProc) {
        printf("need delete\n");
        delete g_svr_task.getVideo.workProc;
        g_svr_task.getVideo.workProc = NULL;
    }

    g_svr_task.getVideo.isStart = true;
    g_svr_task.getVideo.workProc = new std::thread([](){
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

            // if (g_svr_ops.svrVideoCb) {
            //     g_svr_ops.svrVideoCb(data, len, pts, encode, frameType);
            // }
            failedCnt = 0;
        }
        g_svr_task.getVideo.isStart = false;
    });

    return 0;
}
int SVR_StopGetVideo(void)
{
    EVENT event;
    event.eventID = EVENT_GET_VIDEO;
    event.argv1 = false;
    event.result = -1;
    sendMsg(&event);

    g_svr_task.getVideo.isStart = false;
    if (g_svr_task.getVideo.workProc) {
        g_svr_task.getVideo.workProc->join();
        delete g_svr_task.getVideo.workProc;
    }
    return 0;
}

int SVR_StartRecvAudio(void)
{
    return 0;
}
int SVR_SndTaklAudio (char *data, int len, unsigned long long pts, int encode, int sampleRate)
{
    return 0;
}
int SVR_StopRecvAudio(void)
{
    return 0;
}

int SVR_Login(bool bLogin);
int SVR_RequestFileDir(void);
int SVR_RequestGetDevInfo(void);
int SVR_RequestStartRec(void);
int SVR_RequestStopRec(void);
int SVR_RequestTakePhoto(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */


static int svr_callback (EVENT *event) 
{
    // printf("%s\n", __func__);
    switch (event->eventID) 
    {
        case EVENT_GET_VIDEO:
            printf("EVENT_GET_VIDEO, argv1:%d\n", event->argv1);
            break;
        case EVENT_GET_AUDIO:
            printf("EVENT_GET_AUDIO, argv1:%d\n", event->argv1);
            break;
        case EVENT_SND_TALK_AUDIO:
            printf("EVENT_SND_TALK_AUDIO, argv1:%d\n", event->argv1);

            break;
        case EVENT_CTL_START_REC:
            printf("EVENT_CTL_START_REC, argv1:%d\n", event->argv1);
            break;
        case EVENT_CTL_TAKE_PHOTO:
            printf("EVENT_CTL_TAKE_PHOTO, argv1:%d\n", event->argv1);

            break;
        case EVENT_CTL_GET_DIR:
            printf("EVENT_CTL_GET_DIR, argv1:%d\n", event->argv1);
            break;
        case EVENT_CTL_GET_DEV_INFO:
            printf("EVENT_CTL_GET_DEV_INFO, argv1:%d\n", event->argv1);
            break;
        case EVENT_CTL_SOS:
            printf("EVENT_CTL_SOS, argv1:%d\n", event->argv1);
            break;
        case EVENT_LOGIN_STATUS:
            printf("EVENT_LOGIN_STATUS, argv1:%d\n", event->argv1);
            break;
        default:
            break;
    }
    // printf("===============\n");
    // if (event->argv1 > 0) {
    //     event->argv1--;
    //     sendMsg(event);
    // }
    return 0;
}

static int svr_GetAudioCb(const char *data, int len, unsigned long long pts, int encode, int sampleRate)
{
    return 0;
}

static FILE* g_video_fp = NULL;
static int svr_GetVideoCb(const char *data, int len, unsigned long long pts, int frameType, int encode)
{
    printf("[%s]len:%d\n", __func__, len);
    if (g_video_fp) {
        fwrite(data, 1, len, g_video_fp);
    }
    return 0;
}
int main()
{
    int ret = 0;
    SVR_Ops ops;
    ops.svrAudioCb = svr_GetAudioCb;
    ops.svrVideoCb = svr_GetVideoCb;
    ops.svrEventCb = svr_callback;

    ret = SVR_Init(&ops);
    if (ret < 0) {
        printf("SVR_Init failed \n");
        return ret;
    }

    int maxCnt = 0;
    long startEventID = EVENT_GET_VIDEO;
    g_video_fp = fopen("svr_save_video_file.main_app", "wb");
    SVR_StartGetVideo();
    while (maxCnt < 30) {
    //     if (startEventID < EVENT_BUTT) {
    //         printf("startEventID:%ld\n", startEventID);
    //         EVENT event;
    //         event.eventID = startEventID;
    //         event.argv1 = 2;
    //         sendMsg(&event);
    //         startEventID++;
    //     }
    // SVR_StartGetVideo();
        maxCnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    fclose(g_video_fp);

    ret = SVR_Deinit();
    if (ret < 0) {
        printf("SVR_Deinit failed \n");
        return ret;
    }

    return 0;
}