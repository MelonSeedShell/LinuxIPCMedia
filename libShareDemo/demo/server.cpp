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
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
static void testLog(const char *func, int line, char *fmt, ...)
{
    va_list args;
    fprintf(stdout, "[%s-%d]:", func, line);
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

#define LOG(fmt, args...) testLog(__func__, __LINE__, fmt, ##args)

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
            LOG("EVENT_GET_VIDEO, argv1:%d\n", event->argv1);
            break;
        case EVENT_GET_AUDIO:
            LOG("EVENT_GET_AUDIO, argv1:%d\n", event->argv1);
            break;
        case EVENT_SND_TALK_AUDIO:
            LOG("EVENT_SND_TALK_AUDIO, argv1:%d\n", event->argv1);

            break;
        case EVENT_CTL_START_REC:
            LOG("EVENT_CTL_START_REC, argv1:%d\n", event->argv1);
            break;
        case EVENT_CTL_TAKE_PHOTO:
            LOG("EVENT_CTL_TAKE_PHOTO, argv1:%d\n", event->argv1);

            break;
        case EVENT_CTL_GET_DIR:
            LOG("EVENT_CTL_GET_DIR, argv1:%d\n", event->argv1);
            break;
        case EVENT_CTL_GET_DEV_INFO:
            LOG("EVENT_CTL_GET_DEV_INFO, argv1:%d\n", event->argv1);
            break;
        case EVENT_CTL_SOS:
            LOG("EVENT_CTL_SOS, argv1:%d\n", event->argv1);
            break;
        case EVENT_LOGIN_STATUS:
            LOG("EVENT_LOGIN_STATUS, argv1:%d\n", event->argv1);
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
    LOG("[%s:%d]len:%d\n", __func__, __LINE__, len);
    if (g_audio_fp) {
        fwrite(data, 1, len, g_audio_fp);
    }
    return 0;
}

static int svr_GetVideoCb(const char *data, int len, unsigned long long pts, int frameType, int encode)
{
    LOG("[%s:%d]len:%d\n", __func__, __LINE__, len);
    if (g_video_fp) {
        fwrite(data, 1, len, g_video_fp);
    }
    return 0;
}

static int svr_startSendTalkAudio(void)
{
    std::lock_guard<std::mutex> lock(g_talk_task.mtx);
    if (g_talk_task.isStart) {
        LOG("has started\n");
        return 0;
    }
    g_talk_task.isStart = true;
    g_talk_task.workProc = new std::thread([](){
        FILE* fp = fopen("test.pcm", "rb");
        if (!fp) {
            LOG("open test.pcm failed \n");
            return ;
        }

        char data[320 * 2];
        while (g_talk_task.isStart) {
            size_t readLen = fread(data, 1, sizeof(data), fp);
            if (readLen <= 0) {
                LOG("fread failed, exit\n");
                break;
            }
            struct timeval tv;
            gettimeofday(&tv, NULL);
            unsigned long long frame_pts = (tv.tv_sec * 1000 + tv.tv_usec/1000) * 90;
            SVR_SndTalkAudio (data, readLen, frame_pts, AUDIO_ENCODE_PCM, 8000);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        g_talk_task.isStart = false;
    });
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
    // SVR_StartGetVideo();
    // SVR_StartGetAudio();
    // svr_startSendTalkAudio();
    while (g_talk_task.isStart) {
        maxCnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    fclose(g_video_fp);
    fclose(g_audio_fp);

    ret = SVR_Deinit();
    if (ret < 0) {
        printf("SVR_Deinit failed \n");
        return ret;
    }

    return 0;
}