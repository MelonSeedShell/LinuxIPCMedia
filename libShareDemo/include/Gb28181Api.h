#ifndef _Included_gb28181_H
#define _Included_gb28181_H

#ifdef __cplusplus
extern "C" {
#endif
typedef int (*GBCallback)(int iType, const char* szMessage, int iMsgLen, void* pUserParam, int idx);
typedef int (*GBTalkDataCallback)(const unsigned char* szData, int iLen, int iSampleRate, int iSampleSize,
    int iChannels, long lTimeStamp, void* pUserParam,int idx);

int GBStartUp(const char* pszPlatformInfo,int idx);
void GBShutDown(int idx);

int GBGetRegisterStatus(int idx);
bool GBSetVideoInfo(int encode_type,int width, int height, int frame_rate,
                                             unsigned char* pExtraData, int extra_len,int idx);
int GBPushRealTimeVideoFrame(int i_frame_type,int i_encode_type, unsigned char* pRawData,
                                                       int i_length, int i_width, int i_height,
                                                       int i_frame_rate,int idx,unsigned long long pts);
int GBPushRealTimeAudioFrame(int i_frame_type,int i_encode_type, unsigned char* byte_data,
                                                       int i_length, int i_samples_per_sec,
                                                       int i_bits_per_sample, int i_channel_num,
                                                       int idx);
int GBUpdateGPS(const char* pszLat,const char* pszLng,int idx);
int GBSetMsgCallback(GBCallback callback,int idx);
int GBSetGBTalkDataCallback(GBTalkDataCallback callback,int idx);

#ifdef __cplusplus
}
#endif

#endif
