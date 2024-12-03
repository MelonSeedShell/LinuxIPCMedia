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

#if 0
class client
{
public:
    using cliEventCb = std::function<int(EVENT *pEvent)>;
    using cliVidCb = std::function<void(char* data, int& len, unsigned long long& pts, int& encode, int& frameType)>;
public:
    client(/* args */);
    ~client();
    int init(const cliEventCb& eventCb, const cliVidCb& vidCb);
    int deinit();
    int start();
    int stop();

private:
    Msg* m_msg = nullptr;
    ShareVideo* m_shareVid = nullptr;

    cliEventCb m_eventCb = nullptr;
    cliVidCb m_vidCb = nullptr;
};

int client::init(const cliEventCb& eventCb, const cliVidCb& vidCb)
{
    int ret = 0;
    m_eventCb = eventCb;
    m_msg = new Msg([this](const MsgContent& msg){
        EVENT event;
        event.eventID = (unsigned int)msg.msgId;
        event.argv1 = msg.arg1;
        event.result = msg.arg2;

        auto now = std::chrono::system_clock::now();
        // 将时间点转换为毫秒级的时间戳
        auto duration = now.time_since_epoch();
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        long long milliseconds_since_epoch = milliseconds.count();
        event.createTime = milliseconds_since_epoch;
        memcpy(event.aszPayload, msg.payload, sizeof(event.aszPayload));
        m_eventCb(&event);
    });

    ret = m_msg->init();
    if (ret < 0) {
        std::cerr << "cli init failed , msg init failed" << std::endl;
        return -1;
    }
    
    m_vidCb = vidCb;

    m_shareVid = new ShareVideo( 70 << 10);
    ret = m_shareVid->init();
    if (ret < 0) {
        std::cerr << "cli init failed , vid init failed" << std::endl;
        return -1;
    }

    return 0;
}
int client::deinit()
{
    int ret = 0;
    if (m_shareVid) {
        ret = m_shareVid->deinit();
        if (ret < 0) {
            std::cerr << "cli deinit failed , vid deinit failed" << std::endl;
            return -1;
        }
        delete m_shareVid;
    }

    if (m_msg) {
        ret = m_msg->deinit();
        if (ret < 0) {
            std::cerr << "cli deinit failed , msg deinit failed" << std::endl;
            return -1;
        }
        delete m_msg;
    }

    return 0;
}
int client::start()
{
    if (!m_msg || !m_shareVid) {
        std::cerr << "cli start failed , not init" << std::endl;
        return -1;
    }

    if (m_vidCb) {
        
    }
}
int client::stop();

client::client(/* args */)
{
}

client::~client()
{
}
#endif

static Msg* g_msg = nullptr;
static EVTHUB_EVENTPROC_FN_PTR g_eventCb = nullptr;
static ShareVideo* g_shareVid = nullptr;
static ShareAudio* g_shareAud = nullptr;
static ShareTalk* g_shareTalk = nullptr;

static int init(EVTHUB_EVENTPROC_FN_PTR OnEventFn)
{
    int ret = 0;
    g_eventCb = OnEventFn;
    g_msg = new Msg([](const MsgContent& msg){
        EVENT event;
        event.eventID = (unsigned int)msg.msgId;
        event.argv1 = msg.arg1;
        event.result = msg.arg2;
        event.createTime = msg.time;
        memcpy(event.aszPayload, msg.payload, sizeof(event.aszPayload));
        if (g_eventCb) {
            g_eventCb(&event);
        }
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
    g_eventCb = nullptr;
    return 0;
}

static int sendAudio(char *data, int len, unsigned long long pts, int encode, int sampleRate)
{

    return 0;
}

static int sendVideo(char *data, int len, unsigned long long pts, int frameType, int encode)
{
    int ret = 0;
    if (!g_shareVid) {
        return -1;
    }

    ret = g_shareVid->send(data, len, pts, frameType, encode);
    return ret;
}

static int getTalkAudio(char *data, int len, unsigned long long *pts, int *encode, int *sampleRate)
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

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*  __cplusplus  */

int CLI_Init(EVTHUB_EVENTPROC_FN_PTR OnEventFn)
{
    return init(OnEventFn);
}
int CLI_Deinit()
{
    return deinit();
}

int CLI_SndAudio (char *data, int len, unsigned long long pts, int encode, int sampleRate)
{
    return sendAudio(data, len, pts, encode, sampleRate);
}
int CLI_SndVIDEO (char *data, int len, unsigned long long pts, int frameType, int encode)
{
    return sendVideo(data, len, pts, frameType, encode);
}
int CLI_GetTalkAudio (char *data, int len, unsigned long long *pts, int *encode, int *sampleRate)
{
    return getTalkAudio(data, len, pts, encode, sampleRate);
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

typedef struct {
    demo_task sendVideo;
    demo_task sendAudio;
    demo_task getTalk;
} cli_task;
static cli_task g_cli_task;
static int cli_read_and_send_h265video(void)
{
    std::lock_guard<std::mutex> lock(g_cli_task.sendVideo.mtx);
    if (g_cli_task.sendVideo.isStart) {
        printf("has already send video\n");
        return 0;
    }

    if (g_cli_task.sendVideo.workProc) {
        printf("need delete\n");
        delete g_cli_task.sendVideo.workProc;
        g_cli_task.sendVideo.workProc = NULL;
    }

    g_cli_task.sendVideo.workProc = new std::thread([](){
        int ret = 0;
        g_cli_task.sendVideo.isStart = true;
        std::string file = "main_app";
        FILE* fp = fopen(file.c_str(), "rb");
        if (!fp) {
            printf("no such file:%s\n", file.c_str());
            return ;
        }

        FILE* sendSaveFp = fopen("cli_send_save", "wb");

        int autoAfterSendFailedmaxCnt = 10;
        int sendFailedCnt = 0;

        int dataLen = 1024 * 8;
        char* data = new char[dataLen];
        while (g_cli_task.sendVideo.isStart) {
            size_t readLen = fread(data, 1, dataLen, fp);
            if (readLen <= 0) {
                printf("fread failed, exit\n");
                break;
            }

            ret = CLI_SndVIDEO(data, readLen, 0, 0, 0);
            if (ret < 0) {
                printf("CLI_SndVIDEO failed\n");
                if (autoAfterSendFailedmaxCnt < sendFailedCnt) {
                    printf("CLI_SndVIDEO failed cnt out max, exit\n");
                    break;
                }
                sendFailedCnt ++;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (sendSaveFp) {
                fwrite(data, 1, readLen, sendSaveFp);
            }
            sendFailedCnt = 0;
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
    g_cli_task.sendVideo.isStart = false;
    if (g_cli_task.sendVideo.workProc) {
        g_cli_task.sendVideo.workProc->join();
        delete g_cli_task.sendVideo.workProc;
        g_cli_task.sendVideo.workProc = NULL;
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


int main()
{
    int ret = 0;
    ret = CLI_Init(cli_callback);
    if (ret < 0) {
        perror("CLI_Init failed\n");
        return -1;
    }

    int maxCnt = 0;

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