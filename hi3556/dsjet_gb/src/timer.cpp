
#include <stdlib.h>
#include <stdio.h>
#include "Gb28181Api.h"
#include <unistd.h>
#include <string.h>

const char *strPlatformIP = "132.232.209.213";
int nPlatformPort = 5060;
const char * strPlatformCode = "44010200492000000001";
const char * strDeviceDomainName = "4401020049";
const char * strDeviceCode = "44010200492000000031";
const char * strAlarmCode = "44010200492000000031";

const char * strUserName = "admin";
const char * strPassword = "admin123";
const char * mModelName = "F1H";
int mDevListenPort = 15060;
int mPTZType = 0;//0:"未知" 1: "球机" 2: "半球" 3: "固定枪机" 4:"遥控枪机"
int nTalkBackProtocol = 0; //0 ==> UDP; 1 ==> TCP 2://
int workTime = 300;

#define GBT_MSG_OFFLINE 0 // 离线(注册失败/注销成功)
#define GBT_MSG_ONLINE 1 // 在线(注册成功/注销失败)
#define GBT_MSG_REGISTERING 2 // 正在注册
#define GBT_MSG_UNREGISTERING 3 // 正在注销

#define GBT_MSG_SUBSCRIBE_ALARM 4 // 服务器订阅告警消息
#define GBT_MSG_UNSUBSCRIBE_ALARM 5 // 服务器取消订阅告警消息
#define GBT_MSG_SUBSCRIBE_MOBILE_POSITION 6 // 服务器订阅移动位置消息
#define GBT_MSG_UNSUBSCRIBE_MOBILE_POSITION 7 // 服务器取消订阅移动位置消息
#define GBT_MSG_START_REALTIME 8 // 服务器拉取实时视频
#define GBT_MSG_STOP_REALTIME 9 // 服务器关闭实时视频
#define GBT_MSG_START_TALK 10 // 服务器启动对讲
#define GBT_MSG_STOP_TALK  11 // 服务器关闭对讲
#define GBT_MSG_QUERY_RECORD 12 // 服务器查询录像
#define GBT_MSG_PLAYBACK_RECORD 13 // 服务器回放录像
#define GBT_MSG_PTZCMD 14
#define GBT_MSG_CAMERA_SNAP 15
#define GBT_MSG_PLAYBACK_STOP 16 // 服务器回放录像
#define GBT_MSG_Request_History_Alarm 17
#define GBT_MSG_Request_History_Video 18
#define GBT_MSG_Request_GetResidualPower 19
#define GBT_MSG_Request_GetOsdConfig 20
#define GBT_MSG_Request_SetOsdConfig 21
#define GBT_MSG_Request_SetPlatformAddress 22

const static int FRAME_TYPE_I = 0;
const static int FRAME_TYPE_P = 1;
const static int FRAME_TYPE_A = 2;

const static int ENCODE_TYPE_H264 = 0;
const static int ENCODE_TYPE_H265 = 1;
const static int ENCODE_TYPE_PCM = 2;
const static int ENCODE_TYPE_G711A = 3;
const static int ENCODE_TYPE_AAC = 4;

/*
static void sendVideo()
{
    char[] buf;
    int videoFrameRate = 25;
    //sps
    if((buf[3] == 0x01) && (buf[4] == 0x67))
    {
        GBSetVideoInfo(ENCODE_TYPE_H264, mUploadWidth, mUploadHeight, videoFrameRate, buf, buf.length,0);
    }
    //pps
    else if((buf[3] == 0x01) && (buf[4] == 0x68))
    {
        GBSetVideoInfo(ENCODE_TYPE_H264, mUploadWidth, mUploadHeight, videoFrameRate, buf, buf.length,0);
    }
    //i frame
    else if (buf[4] == 0x65)
    {
        GBPushRealTimeVideoFrame(FRAME_TYPE_I, ENCODE_TYPE_H264, buf, buf.length, mUploadWidth, mUploadHeight, videoFrameRate,0);
    }
    //p frame
    else
    {
        GBPushRealTimeVideoFrame(FRAME_TYPE_P, ENCODE_TYPE_H264, buf, buf.length, mUploadWidth, mUploadHeight, videoFrameRate,0);
    }
}

    GBPushRealTimeAudioFrame(FRAME_TYPE_A, ENCODE_TYPE_PCM, mRecordBuffer, readBytes,
                            8000, 16, 1,0);
*/
int Callback(int iType, const char* szMessage, int iMsgLen, void* pUserParam, int idx)
{
    printf("\n %s,iType: %d ,szMessage: %s ,iMsgLen: %d idx:%d \n",__FUNCTION__,iType,szMessage,iMsgLen,idx);
    switch(iType)
    {
        case GBT_MSG_ONLINE:
            printf("GBT_MSG_ONLINE \n");
            break;
        case GBT_MSG_START_REALTIME:
            printf("GBT_MSG_START_REALTIME \n");
            break;
        case GBT_MSG_STOP_REALTIME:
            printf("GBT_MSG_STOP_REALTIME \n");
            break;
    }
    return 0;
}
int TalkDataCallback(const unsigned char* szData, int iLen, int iSampleRate, int iSampleSize,
    int iChannels, long lTimeStamp, void* pUserParam,int idx)
{
    printf("\n %s,iLen: %d ,iSampleRate: %d ,iChannels: %d idx:%d \n",__FUNCTION__,iLen,iSampleRate,iChannels,idx);
    return 0;
}
int dsjet_gb_start(void)
{
    char *param = new char[1024];
    FILE *pFile = NULL;
    pFile = fopen("/proc/cpuinfo", "r");
    if(pFile != NULL) {
        fread(param,1,1024,pFile);
        printf("cpuinfo: %s \n",param);
        memset(param,0,1024);
        fclose(pFile);
    }
    printf("hello\n");
    snprintf(param,1024,"{\"Platform\":{" \
                "\"nEnable\":\"1 \"," \
                "\"nConnectType\":\"1\"," \
                "\"strPlatformIP\":\"%s\"," \
                "\"nPlatformPort\":\"%d\"," \
                "\"strPlatformCode\":\"%s\"," \
                "\"strDevDomainID\":\"%s\"," \
                "\"strDevCode\":\"%s\"," \
                "\"strAlarmCode\":\"%s\"," \
                "\"strUserName\":\"%s\"," \
                "\"strUserID\":\"%s\"," \
                "\"strPassword\":\"%s\"," \
                "\"strModel\":\"%s\"," \
                "\"nDevListenPort\":\"%d\"," \
                "\"nExpiresTime\":\"3600\"," \
                "\"nKeepAliveTime\":\"10\"," \
                "\"nCameraCount\":\"1\"," \
                "\"nChnlType\":\"0\"," \
                "\"mPTZType\":\"%d\"," \
                "\"nChnlNo\":\"1\"," \
                "\"strChnlID01\":\"%s\"," \
                "\"strChnlName01\":\"AVChannel1\"," \
                "\"nAlarmInCount\":\"1\"," \
                "\"nAlarmInChnlType\":\"1\"," \
                "\"nAlarmInChnlNo\":\"1\"," \
                "\"strAlarmInChnlID\":\"%s\"," \
                "\"strAlarmInChnlName\":\"AlarmIn 01\"," \
                "\"nTalkBackProtocol\":\"%d\"," \
                "\"nGPSInterval\":\"15\"}}",strPlatformIP,nPlatformPort,
                strPlatformCode,strDeviceDomainName,strDeviceCode,strDeviceCode,
                strUserName,strDeviceCode,strPassword,mModelName,mDevListenPort,
                mPTZType,strDeviceCode,strAlarmCode,nTalkBackProtocol);
    printf("GBStartUp \n");
    GBSetMsgCallback(Callback,0);
    GBSetGBTalkDataCallback(TalkDataCallback,0);

    GBStartUp(param,0);
    int status = 0;
    while(workTime > 0)
    {
        workTime--;
        usleep(1000*1000);
        status = GBGetRegisterStatus(0);
        printf("status:%d \n",status);
    }
    printf("GBShutDown \n");
    GBShutDown(0);
    delete [] param;
    return 0;
}
