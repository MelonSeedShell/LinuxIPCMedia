#ifndef CR_DEVICE_STRUCT_H
#define CR_DEVICE_STRUCT_H
#pragma once

#include "CR_DeviceNetDef.h"

#ifdef WIN32
#	ifdef CR_SDK_EXPORTS
#		define CR_API __declspec(dllexport)
#	else
#		define CR_API __declspec(dllimport)
#	endif
#else
#define CR_API
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define IN		/* 输入参数 */
#define OUT		/* 输出参数 */

	/* 初始化/逆初始化协议栈 */
	int CR_API CR_Initialize(void);
	int CR_API CR_Terminate(void);

	/*设置设备基本信息，有哪些资源*/
	int CR_API CR_SetDeviceInfo(IN CR_DeviceInfo* pstDeviceInfo);

	/*设置回调函数，各种媒体通知、配置通知、控制通知等*/
	int CR_API CR_SetCtrlHandler(IN CR_CtrlHandler* stCtrlHandler);

	/*连接平台*/
	int CR_API CR_Connect(IN CR_CSTR pszPlatformIP, IN unsigned short usPlatformPort, IN int nFixed, OUT CR_HSESSION *hSession);

	/*退出平台*/
	int CR_API CR_DisConnect(IN CR_HSESSION hSession);

	/*获取连接平台状态，0：表示连接成功， 小于0表示失败（已断开）， 大于0表示正在连接平台*/
	int CR_API CR_GetConStatus(IN CR_HSESSION hSession);

	/*设置当前媒体信息*/
	int CR_API CR_SetMediaInfo(IN CR_MediaInfo* pstMediaInfo);

	/*主动上报报警事件信息*/
	int CR_API CR_UploadAlarm(IN CR_HSESSION hSession, IN CR_AlarmInfo *pAlarmData);

	/*发送定位信息， 返回小于0的值，表示流断开*/
	int CR_API CR_SendGPSData(IN CR_HSTREAM hStream, IN CR_GPSInfo *pDataInfoData,IN int nDataLen);

	/*发送音视频数据（实时、对讲、点播）， 返回小于0的值，表示流断开*/
	int CR_API CR_SendStreamData(IN CR_HSTREAM hStream, IN unsigned char *pData, IN int nFrameLen, IN CR_FRAMETYPE nFrameType, IN int nIsKeyFrame,IN CR_MediaInfo* pMediaInfo,unsigned int uiTimeStamp);

	/*发送下载数据，从文件起始开始读取，不需要关注文件的大小， 返回小于0的值，表示流断开*/
	int CR_API CR_SendDownloadData(IN CR_HSTREAM hStream, IN unsigned char *pData, IN int nDataLen);

	/*主动断开流*/ 
	int CR_API CR_StopStream(IN CR_HSTREAM hStream);

#ifdef __cplusplus
}
#endif

#endif
