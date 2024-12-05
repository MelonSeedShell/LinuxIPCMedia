#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <chrono>
#include <cstdlib>
#include <string.h>
#include <vector>
#include <mutex>
#include <thread>
#include "server.h"


typedef struct {
    bool isStart = false;
    std::thread* workProc = NULL;
    std::mutex mtx;
} demo_task;

#define TEST_SAVE_VIDEO
#define TEST_SAVE_AUDIO
#define TEST_SAVE_TALK
static FILE* g_video_fp = NULL;
static FILE* g_audio_fp = NULL;
static FILE* g_talk_fp = NULL;

static demo_task g_talk_task;

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
    printf("[%s:%d]len:%d\n", __func__, __LINE__, len);
    if (g_audio_fp) {
        fwrite(data, 1, len, g_audio_fp);
    }
    return 0;
}

static int svr_GetVideoCb(const char *data, int len, unsigned long long pts, int frameType, int encode)
{
    printf("[%s:%d]len:%d\n", __func__, __LINE__, len);
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
    g_video_fp = fopen("svr_save_video_file", "wb");
    g_audio_fp = fopen("svr_save_audio_file", "wb");
    SVR_StartGetVideo();
    SVR_StartGetAudio();
    while (maxCnt < 30) {
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