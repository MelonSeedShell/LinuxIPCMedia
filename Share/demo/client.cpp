#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <chrono>
#include <cstdlib>
#include <string.h>
#include <vector>
#include <mutex>
#include "Msg.h"
#include "ShareVideo.h"
#include "ShareTalk.h"
#include "ShareAudio.h"
#include "shareType.h"

typedef int (*GetTalkCb) (const char *data, int len, unsigned long long pts, int encode, int sampleRate);


typedef struct {
    EVTHUB_EVENTPROC_FN_PTR cliEventCb;
    GetTalkCb              cliTalkCb;
} CLI_Ops;

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
    });

    ret = g_msg->init();
    if (ret < 0) {
        std::cerr << "cli init failed , msg init failed" << std::endl;
        return -1;
    }

    g_shareVid = new ShareVideo(70 << 10);
    ret = g_shareVid->init();
    if (ret < 0) {
        std::cerr << "cli init failed , vid init failed" << std::endl;
        return -1;
    }

    g_shareAud = new ShareAudio(2 << 10);
    ret = g_shareAud->init();
    if (ret < 0) {
        std::cerr << "cli init failed , aud init failed" << std::endl;
        return -1;
    }

    g_shareTalk = new ShareTalk(2 << 10);
    ret = g_shareTalk->init();
    if (ret < 0) {
        std::cerr << "cli init failed , talk init failed" << std::endl;
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

    if (g_shareAud) {
        ret = g_shareAud->deinit();
        if (ret < 0) {
            std::cerr << "cli deinit failed , aud deinit failed" << std::endl;
            return -1;
        }
        delete g_shareAud;
        g_shareAud = nullptr;
    }

    if (g_shareTalk) {
        ret = g_shareTalk->deinit();
        if (ret < 0) {
            std::cerr << "cli deinit failed , talk deinit failed" << std::endl;
            return -1;
        }
        delete g_shareTalk;
        g_shareTalk = nullptr;
    }
printf("cli [%s:%d]\n", __func__, __LINE__);
    if (g_msg) {
        ret = g_msg->deinit();
        if (ret < 0) {
            std::cerr << "cli deinit failed , msg deinit failed" << std::endl;
            return -1;
        }
        delete g_msg;
        g_msg = nullptr;
    }
printf("cli [%s:%d]\n", __func__, __LINE__);
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
        std::cerr << "cli get talk failed , talk recv failed" << std::endl;
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
    std::thread* workProc = NULL;
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
    int maxCnt = 10;
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
            continue;
        }

        break;
    }
    return ret;
}
int CLI_SndVIDEO (char *data, int len, unsigned long long pts, int encode, int frameType)
{
    int maxCnt = 10;
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

    if (g_cli_task.getTalk.workProc) {
        printf("need delete\n");
        delete g_cli_task.getTalk.workProc;
        g_cli_task.getTalk.workProc = NULL;
    }
    g_cli_task.getTalk.isStart = true;
    g_cli_task.getTalk.workProc = new std::thread([](){
        int ret = 0;
        int MaxFailedCnt = 10;
        int needReset = 5;
        int failedCnt = 0;
        while (g_cli_task.getTalk.isStart) {
            ret = getTalkAudio();
            if (ret < 0) {
                if (failedCnt > MaxFailedCnt) {
                    printf("getTalkAudio failed cnt out max, exit\n");
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
int CLI_SndMsg(EVENT *event)
{
    return sendMsg(event);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */

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