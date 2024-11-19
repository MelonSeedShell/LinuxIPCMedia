#ifndef CR_DEVICE_NETDEF_H
#define CR_DEVICE_NETDEF_H

#define CR_DEVICENET_VERSION 	"V8.1.2 20230529"

//#define MAX_STR_LEN 1024
typedef	void*			HANDLE;
typedef	void*			CR_HSESSION;			/* 连接句柄 */
typedef	void*			CR_HRESOURCE;			/* 资源句柄 */
typedef	void*			CR_HSTREAM;				/* 流句柄 */
typedef const char*		CR_CSTR;				/* 常量字符串 */
typedef char			CR_STR[1024];	/* 字符串类型 */

//错误码定义
#define CR_DEVICENET_OK					0		//成功
#define CR_DEVICENET_FALSE				-1		//失败
#define CR_DEVICENET_ERR_INVALIDPARAM	-2		//非法参数
#define CR_DEVICENET_ERR_BUFFULL		-3		//缓冲区已满
#define CR_DEVICENET_ERR_EXIST			-4		//对象已存在
#define CR_DEVICENET_ERR_NETWORK		-5		//网络错误
#define CR_DEVICENET_ERR_UNSUPPORT		-6		//属性不支持
#define CR_DEVICENET_ERR_INVALIDRES		-7		//非法资源
#define CR_DEVICENET_ERR_OVERLAP		-8		//不支持交叠操作


enum CR_STREAMTYPE
{
	STREAMTYPE_VIDEO	= 0,	//视频
	STREAMTYPE_AUDIO,		    //音频
	STREAMTYPE_TALK,			//对讲
	STREAMTYPE_GPS,			//定位信息
};

enum CR_STREAMLEVEL_TYPE
{
	STREAMLEVEL_MAIN	= 0,	//主码流
	STREAMLEVEL_SUB,			//辅码流
};

enum CR_FILETYPE
{
	FILETYPE_MEDIA		= 0,	//音视频文件
	FILETYPE_PICTURE,
	FILETYPE_AUDIO,
};

enum CR_FRAMETYPE
{
	FRAMETYPE_VIDEO		= 0,	//视频
	FRAMETYPE_AUDIO,
	FRAMETYPE_PICTURE,
	FRAMETYPE_GPS,
};

enum CR_RESOURCE_TYPE
{
	RESOURCE_TYPE_SELF,
	RESOURCE_TYPE_IV,
	RESOURCE_TYPE_IA,
	RESOURCE_TYPE_OA,
	RESOURCE_TYPE_WM,
	RESOURCE_TYPE_WIFI,
	RESOURCE_TYPE_STORAGE,
	RESOURCE_TYPE_PTZ,
	RESOURCE_TYPE_IDL,
	RESOURCE_TYPE_GPS
};

enum CR_CMDID_DEF
{
	CR_CMD_CFG_NULL		= 0x00000000,	//配置命令
	CR_CMD_FrameRate,                 
	CR_CMD_BitRate,
	CR_CMD_PlatformAddr,
	CR_CMD_CFG_ST_PUID,
	CR_CMD_CFG_ST_RegPsw,
	CR_CMD_SupportedIFPrioritySets,
	CR_CMD_CFG_ST_Model,
	CR_CMD_CFG_ST_SoftwareVersion,
	CR_CMD_CFG_ST_HardwareModel,
	CR_CMD_CFG_ST_HardwareVersion,
	CR_CMD_CFG_ST_ProducerID,
	CR_CMD_CFG_ST_DeviceID,
	CR_CMD_CFG_ST_TZ,
	CR_CMD_CFG_IV_Brightness,
	CR_CMD_CFG_IV_Contrast,
	CR_CMD_CFG_IV_Hue,
	CR_CMD_CFG_IV_Saturation,
	CR_CMD_CFG_IV_ImageDefinition,
	CR_CMD_CFG_OA_Decode,
	CR_CMD_CFG_OA_DecoderProducerID,

	CR_CMD_CTL_NULL		= 0x00100000,	//控制命令
	CR_CMD_CTL_PTZStop,
	CR_CMD_CTL_PTZ_StartTurnLeft,
	CR_CMD_CTL_PTZ_StartTurnRight,
	CR_CMD_CTL_PTZ_StartTurnUp,
	CR_CMD_CTL_PTZ_StartTurnDown,
	CR_CMD_CTL_SG_ManualStart,
	CR_CMD_CTL_SG_ManualStop,
	CR_CMD_CTL_ST_SetTime,
	CR_CMD_CTL_IV_StartRecord,
	CR_CMD_CTL_IV_StopRecord,
	CR_CMD_CTL_SG_QueryRecordFiles,
	CR_CMD_CTL_SG_DelRecordFiles,
	CR_CMD_CTL_IA_AudioStop,
};

enum CR_ALARM_TYPE
{
	SOS_Emergent_Alert,
	DiskSpaceFull_Alert,
};

typedef struct CR_Resoure
{
	int nIdx;
	CR_RESOURCE_TYPE nType;
	char szName[64];
	char szDesc[64];
}tagCRRESOURE;

typedef struct CR_AlarmInfo
{
	char szAlarmID[64];
	int nTime;
	CR_Resoure nResoure;
	CR_ALARM_TYPE ntype;
	void *pDescription;
}tagCRALARMINFO;


//函数句柄
typedef  int  (*CR_StartRealStreamFun)(CR_HSTREAM* hStream, CR_STREAMTYPE nStreamType, CR_STREAMLEVEL_TYPE nStreamLevel, int nIdx);
typedef  int  (*CR_GetParamFun)(CR_CMDID_DEF nCmdID, void* pstRspParam);
typedef  int  (*CR_SetParamFun)(CR_CMDID_DEF nCmdID, void* pstReqParam);
typedef  int  (*CR_CtrlFun)(CR_CMDID_DEF nCmd,void* pstReqParam, void* pstRspParam);
typedef  int  (*CR_StartVodRecordFileFun)(CR_HSTREAM* hStream, char* pFileName, CR_FILETYPE nFileType, int nSpeed, int nDirection);
typedef  int  (*CR_StartDownloadFileFun)(CR_HSTREAM* hStream, char* pFileName);
typedef  int  (*CR_RecvTalkDataFun)(CR_HSTREAM hStream, unsigned char *pFrameData, int nFrameLen, int nAudioAlg, unsigned int uiTimestamp);


typedef struct CR_CtrlHandler
{
	CR_StartRealStreamFun pfStartRealStream;
	CR_RecvTalkDataFun pfRecvTalkData;
	CR_GetParamFun pfGetParam;
	CR_SetParamFun pfetParam;
	CR_CtrlFun pfCtrlFun;
	CR_StartVodRecordFileFun pfVodRecordFile;
	CR_StartDownloadFileFun pfDownloadRecordFile;
}tagCRCTRLHANDLER;

#define CR_MAX_RESOURCE_NUM	64
typedef struct CR_DeviceInfo
{
	int nProdurceID;	//厂商ID
	char szPUID[24];	//设备PUID
	char szDevID[32];	//设备ID
	char szDevType[32];	//设备类型WENC、ENC
	char szMACID[32];	//MAC
	char szIP[32];		//设备IP
	char szPwd[32];		//接入密码
	int nResNum;		//资源数量
	CR_Resoure nResource[CR_MAX_RESOURCE_NUM];
}tagCRDEVICEINFO;

//当前配置的媒体信息
typedef struct CR_MediaInfo
{
	//视频信息
	int nVideoAlg;
	int nFrameRatePerSec;
	int nWidth;
	int nHeigh;

	//音频信息
	int nFrameNum;
	int nAudioAlg;
	int nAudioChannel;//声道数量
	int nAudioSamplesPerSec; //采样率
	int nAudioBitPerSamples;//采样深度

}tagCRMEDIAINFO;

typedef struct CR_GPSInfo
{
	float fLatitude;	// 纬度,取值范围[-90,90],北纬为正,南纬为负
	float fLongitude;	// 经度,现在取值是[-180,180),东经为正,西经为负
	float fBearing;				// 方向,[0,360),正北为0,正东为90,依次类推.
	float fSpeed;				// 速度,单位km/h
	float fAltitude;			// 海拔,单位m,小于等于-50000表示无效
	unsigned int uiTime;		// 时间,单位秒,UTC时间.
	unsigned char ucGNNSType;	// 定位系统类型
	unsigned char ucOfflineFlag;// 是否离线数据: 0表示实时数据;1表示设备下线时存的数据
	unsigned char ucState;		// 定位模块状态: 0表示正常;1表示无信号;2表示无模块
	unsigned short usMaxSpeed;	// 最高限速，短整型，km/h
	unsigned short usMinSpeed;	// 最低限速，短整型，km/h
	unsigned short usMileage;	// 上次上传的数据和这次上传之间的里程
	unsigned short Rsv;			// 保留,填0
}tagCRGPSINFO;

#endif //SGSDK_STRUCT_H
