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
#include "client.h"


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

static demo_task g_video_task;
static demo_task g_audio_task;

static int cli_read_and_send_h265video(void)
{
    std::lock_guard<std::mutex> lock(g_video_task.mtx);
    if (g_video_task.isStart) {
        printf("has already send video\n");
        return 0;
    }

    if (g_video_task.workProc) {
        printf("need delete\n");
        delete g_video_task.workProc;
        g_video_task.workProc = NULL;
    }

    g_video_task.workProc = new std::thread([](){
        int ret = 0;
        g_video_task.isStart = true;
        std::string file = "main_app";
        FILE* fp = fopen(file.c_str(), "rb");
        if (!fp) {
            printf("no such file:%s\n", file.c_str());
            return ;
        }

        FILE* sendSaveFp = fopen("cli_send_save", "wb");
        int dataLen = 1024 * 8;
        char* data = new char[dataLen];
        while (g_video_task.isStart) {
            size_t readLen = fread(data, 1, dataLen, fp);
            if (readLen <= 0) {
                printf("fread failed, exit\n");
                break;
            }

            ret = CLI_SndVIDEO(data, readLen, 0, 0, 0);
            if (ret < 0) {
                printf("CLI_SndVIDEO failed, exit\n");
                break;
            }
            if (sendSaveFp) {
                fwrite(data, 1, readLen, sendSaveFp);
            }
        }

        fclose(fp);
        fclose(sendSaveFp);
        delete[] data;

        // EVENT event;
        // event.eventID = EVENT_GET_VIDEO;
        // event.argv1 = false;
        // event.result = -1;
        // CLI_SndMsg(&event);
    });
    return 0;
}
static int cli_stop_send_video(void)
{
    g_video_task.isStart = false;
    if (g_video_task.workProc) {
        g_video_task.workProc->join();
        delete g_video_task.workProc;
        g_video_task.workProc = NULL;
    }

    EVENT event;
    event.eventID = EVENT_GET_VIDEO;
    event.argv1 = false;
    event.result = 0;
    CLI_SndMsg(&event);
    return 0;
}

static int cli_callback (EVENT *event) 
{
    // printf("\n#########\n");
    // printf("%s, event->eventID:%d\n", __func__, event->eventID);
    switch (event->eventID) 
    {
        case EVENT_GET_VIDEO:
            printf("EVENT_GET_VIDEO, argv1:%d\n", event->argv1);
            {
                if ((bool)event->argv1 == true) {//
                    cli_read_and_send_h265video();
                } else {
                    cli_stop_send_video();
                }
            }
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

    return 0;
}


static int cli_talk_callback(const char *data, int len, unsigned long long pts, int frameType, int encode)
{
    printf("[%s]len:%d\n", __func__, len);
    if (g_talk_fp) {
        fwrite(data, 1, len, g_talk_fp);
    }
    return 0;
}

int main()
{
    int ret = 0;
    CLI_Ops ops;
    ops.cliEventCb = cli_callback;
    ops.cliTalkCb = cli_talk_callback;
    ret = CLI_Init(&ops);
    if (ret < 0) {
        perror("CLI_Init failed\n");
        return -1;
    }

    int maxCnt = 0;
    g_talk_fp = fopen("cli_save_talk_file", "wb");
    // cli_read_and_send_h265video();

    while (maxCnt < 30) {
        maxCnt++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    ret = CLI_Deinit();
    if (ret < 0) {
        perror("CLI_Deinit failed\n");
        return -1;
    }
    printf("end\n");
    return 0;
}