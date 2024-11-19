#include <iostream>
#include <sys/types.h>
#include <cstring>
#include <stdlib.h>
#include "server.h"
#include "CR_DeviceNet.h"
#include "CR_DeviceNetDef.h"
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <map>
#include <vector>

#include <stdarg.h>
#include <stdio.h>
static void testLog(const char *func, int line, char *fmt, ...)
{
    va_list args;
    fprintf(stdout, "[%s-%d]:", func, line);
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

#define LOG(fmt, args...) testLog(__func__, __LINE__, fmt, ##args)


#define DEFAULT_MEDIA_SVR_CFG  "/app/DefaultMediaSvr.cfg"
#define MEDIA_SVR_CFG          "/app/sd/MediaSvr.cfg"

// #define TEST_VIDEO
// #define TEST_AUDIO
// #define TEST_TALK
#ifdef TEST_TALK
static FILE *g_talkFp = NULL;
#endif
typedef struct dev_info {
    char szUser[32] ;
    char szPwd[32] ;
    char szIpAddr[16] ;
    unsigned int  u32Port ;
    char szSvid[32] ;
    char szCid[32] ;
    char szVgbid[32] ;
    char szVgname[32] ;
    int  gpsFormat ;
    int  gpsUploadTime ;
    double lng;
    double lat;
} DEV_INFO;
static DEV_INFO g_DevInfo;
static bool bGpsUpload = false;
static bool bOnPTT = false;
static CR_DeviceInfo g_CsConfig ;
static CR_HSTREAM g_HandleVideo = NULL;
static CR_HSTREAM g_HandleAudio = NULL;
static CR_HSTREAM g_HandleGps = NULL;
static CR_MediaInfo g_AudioMediaInfo;
static CR_MediaInfo g_VideoMediaInfo;

static CR_HSESSION g_hConnectHandle;
static CR_CtrlHandler g_stCrCtrlOpt;

std::vector<std::vector<std::string>> paramVector = {
    {"DeviceID", "DeviceID=\"DeviceID111\""},
    {"Password", "Password=\"000000\""},
    {"DeviceType", "DeviceType=\"WENC\""},
    {"MacID", "MacID=\"E30DD991259B\""},
    {"PUID", "PUID=\"151120111122223355\""},
    {"ServerIP", "ServerIP=\"61.191.26.2\""},
    {"ServerPort", "ServerPort=\"20016\""},
    {"ConnectMode", "ConnectMode=\"1\""},
};
std::map<std::string, std::string> ParamMap;


static std::string getJsonStringVal(const char* msg, const char* key)
{
	std::string method;
	const char* pS = msg;
	const char* pE = 0;
	std::string tmpKey;
	tmpKey = key;
	tmpKey += "\":";

	pS = strstr(msg, tmpKey.c_str());
	if (pS != 0)
	{
		pE = strstr(pS, "\",");
		if (pE != 0)
		{
			char tmp[64] = { 0 };
			memcpy(tmp, pS + strlen(key) + 3, pE - pS - (strlen(key) + 3));
			method = tmp;
		}
	}

	return method;
}

static int getJsonIntVal(const char* msg, const char* key)
{
	const char* pS = msg;
	const char* pE = 0;
	int ret = 0;
	std::string tmpKey;
	tmpKey = key;
	tmpKey += "\":";

	pS = strstr(msg, tmpKey.c_str());
	if (pS != 0)
	{
		pE = strstr(pS, ",");
		if (pE != 0)
		{
			char tmp[64] = { 0 };
			memcpy(tmp, pS + strlen(key) + 2, pE - pS - (strlen(key) + 2));
			ret = atoi(tmp);
		}
	}
	return ret;
}

static int GetDevInfo(const char *pJSON)
{
    LOG("\n");
    int port;
    int gpsFormat;
    int gpsUploadTime;
    std::string user, pwd, ip, svid, cid, vgbid, vgbname;
    std::string lng, lat;
    user = getJsonStringVal(pJSON, "User");
    pwd = getJsonStringVal(pJSON, "Pwd");
    ip = getJsonStringVal(pJSON, "IpAddr");
    port = getJsonIntVal(pJSON, "Port");
    svid = getJsonStringVal(pJSON, "Svid");
    cid = getJsonStringVal(pJSON, "Cid");
    vgbid = getJsonStringVal(pJSON, "Vgbid");
    vgbname = getJsonStringVal(pJSON, "Vgname");

    gpsFormat = getJsonIntVal(pJSON, "GpsFormat");
    gpsUploadTime = getJsonIntVal(pJSON, "GpsUploadTime");

    lng = getJsonStringVal(pJSON, "lng");
    lat = getJsonStringVal(pJSON, "lat");
    // LOG("user:%s\n", user.c_str());
    // LOG("pwd:%s\n", pwd.c_str());
    // LOG("ip:%s\n", ip.c_str());
    // LOG("port:%d\n", port);
    // LOG("svid:%s\n", svid.c_str());
    // LOG("cid:%s\n", cid.c_str());
    // LOG("vgbid:%s\n", vgbid.c_str());
    // LOG("vgbname:%s\n", vgbname.c_str());
    // LOG("lng:%s\n", lng.c_str());
    // LOG("lat:%s\n", lat.c_str());
    snprintf(g_DevInfo.szUser, sizeof(g_DevInfo.szUser), "%s", user.c_str());
    snprintf(g_DevInfo.szPwd, sizeof(g_DevInfo.szPwd), "%s", pwd.c_str());
    snprintf(g_DevInfo.szIpAddr, sizeof(g_DevInfo.szIpAddr), "%s", ip.c_str());
    g_DevInfo.u32Port = port;
    snprintf(g_DevInfo.szSvid, sizeof(g_DevInfo.szSvid), "%s", svid.c_str());
    snprintf(g_DevInfo.szCid, sizeof(g_DevInfo.szCid), "%s", cid.c_str());
    snprintf(g_DevInfo.szVgbid, sizeof(g_DevInfo.szVgbid), "%s", vgbid.c_str());
    snprintf(g_DevInfo.szVgname, sizeof(g_DevInfo.szVgname), "%s", vgbname.c_str());

    g_DevInfo.gpsFormat = gpsFormat;
    g_DevInfo.gpsUploadTime = gpsUploadTime;

    g_DevInfo.lng = strtod(lng.c_str(), NULL);
    g_DevInfo.lat = strtod(lat.c_str(), NULL);

    // g_DevInfo.bUdp = true;
    LOG("\n");
    return 0;

}

static int GetTimeStamp(uint64_t *timeStamp)
{
    auto now = std::chrono::system_clock::now();
    *timeStamp = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    return 0;
}

static void *UploadGpsThread(void *argv)
{
    int ret = 0;
    uint64_t timeStamp;
    int wait_time = 0;
    CR_GPSInfo stCsGpsInfo;
    memset(&stCsGpsInfo, 0, sizeof(stCsGpsInfo));
    while (bGpsUpload) {
        // SVR_RequestGetDevInfo();
        sleep(2);
        GetTimeStamp(&timeStamp);

        stCsGpsInfo.fLatitude = g_DevInfo.lat;
        stCsGpsInfo.fLongitude = g_DevInfo.lng;
        stCsGpsInfo.uiTime = (unsigned int)timeStamp;

        // ret = CR_SendGPSData(g_HandleGps, &stCsGpsInfo, sizeof(stCsGpsInfo));
        if (ret < 0) {
            break;
        }
        wait_time = g_DevInfo.gpsUploadTime - 2 <= 0 ? 1 : g_DevInfo.gpsUploadTime - 2;
        sleep(wait_time);
    }
    bGpsUpload = false;
    return nullptr;
}
static int CS_StartUploadGps(void)
{
    if (bGpsUpload) {
        return 0;
    }
    bGpsUpload = true;
    pthread_t pGpsUploadThread;
    return pthread_create(&pGpsUploadThread, nullptr, UploadGpsThread, nullptr);
}

static int CS_StopGps()
{
    if (!bGpsUpload) {
        return 0;
    }

    bGpsUpload = false;
    return 0;
}

static int CS_SOS(void)
{
	CR_AlarmInfo tSosAlarmInfo;
	memset(&tSosAlarmInfo, 0, sizeof(CR_AlarmInfo));
	tSosAlarmInfo.nResoure.nIdx = 0;
	tSosAlarmInfo.nResoure.nType = RESOURCE_TYPE_SELF;
	strcpy(tSosAlarmInfo.nResoure.szName, "SELF");
	strcpy(tSosAlarmInfo.nResoure.szDesc, "SELF");
	tSosAlarmInfo.nTime = time(NULL);
	tSosAlarmInfo.ntype = SOS_Emergent_Alert;

	// int ret = CR_UploadAlarm(g_hConnectHandle, &tSosAlarmInfo);
    // LOG("ret:%d \n", ret);

	return 0;
}


int StartVodRecordFileCallBack(CR_HSTREAM* hStream, char* pFileName, CR_FILETYPE nFileType, int nSpeed, int nDirection)
{LOG("\n");
	return 0;
}

int StartDownloadFileCallBack(CR_HSTREAM* hStream, char* pFileName)
{LOG("\n");
	return 0;
}

int RecvTalkDataCallBack(CR_HSTREAM hStream, unsigned char *pFrameData, int nFrameLen, int nAudioAlg, unsigned int uiTimestamp)
{
    int ret = 0;
#ifdef TEST_TALK
    if (g_talkFp != NULL) {
        // LOG("nFrameLen:%d \n", nFrameLen);
        ret = fwrite(pFrameData, 1, nFrameLen, g_talkFp);
        if (ret <= 0) {
            LOG("fwrite data failed \n");
        }
    }
#endif

    ret = SVR_SndTaklAudio((char*)pFrameData, nFrameLen, uiTimestamp, 0, 8000);
    if (ret < 0) {
        LOG("SVR_SndTaklAudio failed \n");
        if (ret == -2) {// need start recv audio
            SVR_StartRecvAudio();
        }
    }

	return 0;
}

int SetParamCallBack(CR_CMDID_DEF nCmdID, void* pstReqParam)
{
	LOG("SetParamCallBack Process Set Param!!\n");
	if (pstReqParam == NULL)
	{
		return 0;
	}

	switch (nCmdID)
	{
	case CR_CMD_FrameRate:
		// m_pParamImpl->m_uiFrameRate = atoi((char*)pstReqParam);
		break;
	case CR_CMD_BitRate:
		// m_pParamImpl->m_uiBitRate = atoi((char*)pstReqParam);
		break;
	case CR_CMD_PlatformAddr:
/*		{
			char *pPositon = strstr((char*)pstReqParam, ":");
			if (pPositon != NULL)
			{
				m_pParamImpl->m_strPlatformIP = std::string((char*)pstReqParam, (int)(pPositon-(char*)pstReqParam));
				m_pParamImpl->m_strPlatformPort = std::string(pPositon+1);
			}
		}
*/
		break;
	case CR_CMD_CFG_ST_PUID:

		break;
	case CR_CMD_CFG_ST_RegPsw:

		break;
	case CR_CMD_SupportedIFPrioritySets:

		break;
	case CR_CMD_CFG_ST_Model:

		break;
	case CR_CMD_CFG_ST_SoftwareVersion:
		// m_pParamImpl->m_strSoftVersion = (char*)pstReqParam;
		break;
	case CR_CMD_CFG_ST_HardwareModel:

		break;
	case CR_CMD_CFG_ST_HardwareVersion:
		// m_pParamImpl->m_strHardVersion = (char*)pstReqParam;
		break;
	case CR_CMD_CFG_ST_ProducerID:

		break;
	case CR_CMD_CFG_ST_DeviceID:

		break;
	case CR_CMD_CFG_ST_TZ:

		break;
	case CR_CMD_CFG_IV_Brightness:
		// m_pParamImpl->m_uiBrightness = atoi((char*)pstReqParam);
		break;
	case CR_CMD_CFG_IV_Contrast:
		// m_pParamImpl->m_uiContrast = atoi((char*)pstReqParam);
		break;
	case CR_CMD_CFG_IV_Hue:

		break;
	case CR_CMD_CFG_IV_Saturation:
		// m_pParamImpl->m_uiSaturation = atoi((char*)pstReqParam);
		break;
	case CR_CMD_CFG_IV_ImageDefinition:
		// m_pParamImpl->m_uiImageDef = atoi((char*)pstReqParam);
		break;
	case CR_CMD_CFG_OA_Decode:
		// m_pParamImpl->m_strOADecode = (char*)pstReqParam;
		break;
	case CR_CMD_CFG_OA_DecoderProducerID:
		// m_pParamImpl->m_strDecoderProducerID = (char*)pstReqParam;
		break;
	default:

		break;
	}

	return 0;
}

int GetParamCallBack(CR_CMDID_DEF nCmdID, void* pstRspParam)
{
	LOG("GetParamCallBack Process Get Param!!\n");
	switch (nCmdID)
	{
	case CR_CMD_FrameRate:
		// sprintf((char*)pstRspParam, "%d", m_pParamImpl->m_uiFrameRate);
		break;
	case CR_CMD_BitRate:
		// sprintf((char*)pstRspParam, "%d", m_pParamImpl->m_uiBitRate);
		break;
	case CR_CMD_PlatformAddr:
		// sprintf((char*)pstRspParam, "%s:%s", m_pParamImpl->m_strPlatformIP.c_str(), m_pParamImpl->m_strPlatformPort.c_str());
		break;
	case CR_CMD_CFG_ST_PUID:

		break;
	case CR_CMD_CFG_ST_RegPsw:

		break;
	case CR_CMD_SupportedIFPrioritySets:

		break;
	case CR_CMD_CFG_ST_Model:

		break;
	case CR_CMD_CFG_ST_SoftwareVersion:
		// sprintf((char*)pstRspParam, "%s", m_pParamImpl->m_strSoftVersion.c_str());
		break;
	case CR_CMD_CFG_ST_HardwareModel:

		break;
	case CR_CMD_CFG_ST_HardwareVersion:
		// sprintf((char*)pstRspParam, "%s", m_pParamImpl->m_strHardVersion.c_str());
		break;
	case CR_CMD_CFG_ST_ProducerID:

		break;
	case CR_CMD_CFG_ST_DeviceID:

		break;
	case CR_CMD_CFG_ST_TZ:

		break;
	case CR_CMD_CFG_IV_Brightness:
		// sprintf((char*)pstRspParam, "%d", m_pParamImpl->m_uiBrightness);
		break;
	case CR_CMD_CFG_IV_Contrast:
		// sprintf((char*)pstRspParam, "%d", m_pParamImpl->m_uiContrast);
		break;
	case CR_CMD_CFG_IV_Hue:

		break;
	case CR_CMD_CFG_IV_Saturation:
		// sprintf((char*)pstRspParam, "%d", m_pParamImpl->m_uiSaturation);
		break;
	case CR_CMD_CFG_IV_ImageDefinition:
		// sprintf((char*)pstRspParam, "%d", m_pParamImpl->m_uiImageDef);
		break;
	case CR_CMD_CFG_OA_Decode:
		// sprintf((char*)pstRspParam, "%s", m_pParamImpl->m_strOADecode.c_str());
		break;
	case CR_CMD_CFG_OA_DecoderProducerID:
		// sprintf((char*)pstRspParam, "%s", m_pParamImpl->m_strDecoderProducerID.c_str());
		break;
	default:

		break;
	}
	return 0;
}

int CtrlCallBack(CR_CMDID_DEF nCmd,void* pstReqParam, void* pstRspParam)
{
	LOG("CtrlCallBack Process Ctl Cmd Param!!\n");
	switch (nCmd)
	{
	case CR_CMD_CTL_PTZStop:
	case CR_CMD_CTL_PTZ_StartTurnLeft:
	case CR_CMD_CTL_PTZ_StartTurnRight:
	case CR_CMD_CTL_PTZ_StartTurnUp:
	case CR_CMD_CTL_PTZ_StartTurnDown:

		break;
	case CR_CMD_CTL_SG_ManualStart:

		break;
	case CR_CMD_CTL_SG_ManualStop:

		break;
	case CR_CMD_CTL_ST_SetTime:

		break;
	case CR_CMD_CTL_IV_StartRecord:
        // SVR_RequestStartRec();
		break;
	case CR_CMD_CTL_IV_StopRecord:
        // SVR_RequestStopRec();
		break;
	case CR_CMD_CTL_SG_QueryRecordFiles:

		break;
	case CR_CMD_CTL_SG_DelRecordFiles:

		break;
    case CR_CMD_CTL_IA_AudioStop:
        LOG("talk stop\n");
        break;
	default:
		break;
	}

	return CR_DEVICENET_OK;
}

int StartRealStreamCallBack(CR_HSTREAM* hStream, CR_STREAMTYPE nStreamType, CR_STREAMLEVEL_TYPE nStreamLevel, int nIdx)
{
	std::string strStreamType;				// 实时流类型
    CR_RESOURCE_TYPE needResType ;
	switch (nStreamType)
	{
	case STREAMTYPE_VIDEO:
		strStreamType = "IV";
        needResType = RESOURCE_TYPE_IV;
        g_HandleVideo = *hStream;
        SVR_StartGetVideo();
		break;
	case STREAMTYPE_AUDIO:
		strStreamType = "IA";
        needResType = RESOURCE_TYPE_IA;
        g_HandleAudio = *hStream;
        SVR_StartGetAudio();
		break;
	case STREAMTYPE_GPS:
		strStreamType = "GPS";
        needResType = RESOURCE_TYPE_GPS;
        LOG("\n");
        g_HandleGps = *hStream;
        CS_StartUploadGps();
        LOG("\n");
		break;
	case STREAMTYPE_TALK:
		strStreamType = "OA";
        needResType = RESOURCE_TYPE_PTZ;
#ifdef TEST_TALK
        if (g_talkFp == NULL) {
            g_talkFp = fopen("/app/sd/audio/recv.pcm", "wb");
            if (g_talkFp == NULL) {
                LOG("open file failed \n");
                return 0;
            }
        }
#endif
        SVR_StartRecvAudio();
		break;
	}

	return 0;
}



static int StringToKeyValue(std::string &src, std::string &key, std::string &value)
{
    int ret = 0;
    char *szKey = new char[128];
    char *szValue = new char[128];
    memset(szKey, 0, sizeof(szKey));
    memset(szValue, 0, sizeof(szValue));
    ret = sscanf(src.c_str(), "%[^=]=\"%[^\"]", szKey, szValue);
    if (ret != 2) {
        LOG("src not match *=* \n");
        return -1;
    }
    // LOG("szKey:%s \n szValue:%s \n", szKey, szValue);
    key = szKey;
    value = szValue;

    delete[] szKey;
    delete[] szValue;
    return 0;
}
static int StringToParam(std::string &string, std::map<std::string, std::string> &map)
{
    if (string.substr(0, 1) == "#" || string.substr(0, 1) == ";") {
        return 0;//get line is annotation
    }
    std::string key;
    std::string value;
    if (StringToKeyValue(string, key, value) == 0) {
        map[key] = value;
    } else {
        LOG("can't support analysis : %s\n", string.c_str());
        return -1;
    }

    return 1;
}
static int ParamToFile(std::string filePath, const std::vector<std::vector<std::string>>& vector)
{
    std::ofstream file(filePath.c_str(), std::ios::out | std::ios::trunc);
    if (file.is_open()) {
        LOG("open %s success\n", DEFAULT_MEDIA_SVR_CFG);
        // 在这里对新创建的文件进行操作
        int rows = vector.size();
        for (int i = 0; i < rows; i ++) {
            int cols = vector[i].size(); 
            if (cols != 2) {
                break;
            }
            file << vector[i][1]  + "\n";
        }
        file.close();
    } else {
        LOG("can't create file :%s \n", DEFAULT_MEDIA_SVR_CFG);
        return -1;
    }

    return 0;
}
static int LoadCfgByFile(std::string filePath, std::map<std::string, std::string> &map)
{
    int ret = 0;
    int getParamCnt = 0;
    std::ifstream File(filePath);
    if (File.is_open()) {
        // LOG("open %s \n", filePath.c_str());
        std::string line;
        while (std::getline(File, line)) {
            // LOG("cur line is : %s \n", line.c_str());
            ret = StringToParam(line, map);
            if (ret == -1) {
                LOG("file content or format err \n");
                getParamCnt = -1;
                break;
            } else if (ret == 1) {
                getParamCnt ++;
            }
        }
    } else {
        LOG("no such file :%s , need load device default cfg\n", filePath.c_str());
    }

    File.close();
    return getParamCnt;
}
static int SVR_LoadSettingCfg(std::string filePath, std::map<std::string, std::string> &map)
{
    int ret = LoadCfgByFile(filePath, map);
    if (ret <= -1) {
        LOG("load cfg file failed\n");
    } else if (ret == 0) {
        LOG("no get param from file :%s\n", filePath.c_str());
    }
    return ret;
}
static int SVR_LoadDefaultCfg(std::string filePath, const std::vector<std::vector<std::string>>& vector,
    std::map<std::string, std::string> &map)
{
    int ret = 0;
    std::ifstream File(filePath.c_str());
    if (!File) {
        ParamToFile(filePath, vector);
    } else {
        File.close();
    }

    ret = LoadCfgByFile(filePath, map);
    if (ret <= 0) {
        LOG("load cfg failed\n");
        return -1;
    }

    return ret;
}
static int SVR_SaveParamToDefaultCfg(std::string filePath, std::vector<std::vector<std::string>>& vector,
    std::map<std::string, std::string> &map)
{
    int i = 0;
    int rows = vector.size();
    for (i = 0; i < rows; i ++) {
        int cols = vector[i].size(); 
        if (cols != 2) {

            LOG("\n");
            break;
        }
        // LOG("vector[i][0].c_str():%s \n", vector[i][0].c_str());
        auto it = map.find(vector[i][0].c_str());
        // LOG("it->first:%s, it->second:%s\n", it->first.c_str(), it->second.c_str());
        if (it == map.end()) {
            // LOG("\n");
            break;
        }

        vector[i][1] = it->first + "=\"" + it->second + "\"";
    }
    if (i < rows) {
        LOG("err :param incomplete \n");
        return -1;
    }
    return ParamToFile(filePath, vector);
}

static int getValueFromParam(std::string &value, const std::string &key, const std::map<std::string, std::string> &map)
{
    int ret = 0;

    auto it = map.find(key);
    if (it == map.end()) {
        LOG("can't find the key(%s) \n", key.c_str());
        return -1;
    }

    value = it->second;

    return ret;
}

static int pfnGetAudioCB(char *data, int len, unsigned long long pts, int encode, int sampleRate)
{
    // LOG("len:%d, pts:%llu\n");
    g_AudioMediaInfo.nAudioSamplesPerSec = sampleRate;
    // int ret = CR_SendStreamData((CR_HSTREAM*)g_HandleAudio, (unsigned char*)data, len, FRAMETYPE_AUDIO, 0, &g_AudioMediaInfo, pts);

    return 0;
}

static int pfnGetVideoCB(char *data, int len, unsigned long long pts, int frameType, int encode)
{
    // LOG("len:%d, pts:%llu\n");
    bool isKeyFrame ;
    if (encode == VIDEO_ENCODE_H264) {
        if (frameType == H264E_ISLICE || frameType == H264E_BSLICE || frameType == H264E_IDRSLICE) {
            isKeyFrame = 1;
        } else {
            isKeyFrame = 0;
        }
    } else if (encode == VIDEO_ENCODE_H265) {
        if (frameType == H265E_ISLICE || frameType == H265E_BSLICE) {
            isKeyFrame = 1;
        } else {
            isKeyFrame = 0;
        }
    }

    // int ret = CR_SendStreamData((CR_HSTREAM*)g_HandleVideo, (unsigned char*)data, len, FRAMETYPE_VIDEO, isKeyFrame,
    //          &g_VideoMediaInfo, pts);
    int ret = 0;
    if (ret < 0) {
#ifdef TEST_TALK
    if (g_talkFp != NULL) {
        fclose(g_talkFp);
    }
#endif
    }
    return ret;
}

static int pfnEventCB(EVENT *event)
{
    switch (event->eventID)
    {
        case EVENT_CTL_START_REC:
            if(event->result == 0) {
                LOG("start rec success\n");
            }
            break;
        case EVENT_CTL_TAKE_PHOTO:
            LOG("EVENT_CTL_TAKE_PHOTO\n");
            if(event->result == 0) {
                LOG("take photo success\n");
            }
            break;
        case EVENT_CTL_GET_DIR:
            if(event->result == 0) {
                LOG("get dev dir success\n");
                LOG("dir info : \n%s\n", event->aszPayload);
            }
            break;
        case EVENT_CTL_GET_DEV_INFO:
            if(event->result == 0) {
                GetDevInfo(event->aszPayload);
            }
            break;
        case EVENT_CTL_SOS:
            if ((bool)event->argv1 == true) {
                LOG("dev start sos\n");
                CS_SOS();
            } else {
                LOG("dev stop sos\n");
            }

            break;
        case EVENT_CTL_PTT:
            if ((bool)event->argv1 == true) {
                LOG("dev press ptt\n");
                bOnPTT = (bool)event->argv1;
            } else {
                LOG("dev release ptt\n");
                bOnPTT = false;
            }
            break;
    
        default:
            break;
    }

    return 0;
}


extern int dsjet_gb_start(void);
int main(int argc, char const *argv[])
{
    SVR_Ops ops;
    memset(&ops, 0, sizeof(ops));
    ops.svrEventCb = pfnEventCB;
    ops.svrAudioCb = pfnGetAudioCB;
    ops.svrVideoCb = pfnGetVideoCB;
    int ret = SVR_Init(&ops);
    if (ret < 0) {
        return 0;
    }

    dsjet_gb_start();

    return 0;
}
