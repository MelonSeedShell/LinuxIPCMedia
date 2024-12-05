#ifndef __gb28181_Def_H__
#define __gb28181_Def_H__

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


const char *strPlatformIP = "211.139.163.62";
int nPlatformPort = 6608;
const char * strPlatformCode = "44010200495000000001";
const char * strDeviceDomainName = "4401020049";
const char * strDeviceCode = "44010200495000000036";
const char * strAlarmCode = "44010200495000000036";

const char * strUserName = "admin";
const char * strPassword = "ok123456";
const char * mModelName = "F1H";
int mDevListenPort = 15060;
int mPTZType = 0;//0:"未知" 1: "球机" 2: "半球" 3: "固定枪机" 4:"遥控枪机"
int nTalkBackProtocol = 0; //0 ==> UDP; 1 ==> TCP 2://
int workTime = 30;

const int mUploadWidth = 640;
const int mUploadHeight = 480;

bool bGetIFrame = false;
typedef struct {
    unsigned char buf[128];
    unsigned int len;
} H264_NAL_PARAM_BUFFER;

H264_NAL_PARAM_BUFFER sps_pps_Buf;
#endif