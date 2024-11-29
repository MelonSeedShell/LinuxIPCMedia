#pragma once

#include "shareType.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef int (*GetAudioCb) (char *data, int len, unsigned long long pts, int encode, int sampleRate);
typedef int (*GetVideoCb) (char *data, int len, unsigned long long pts, int frameType, int encode);

typedef struct {
    EVTHUB_EVENTPROC_FN_PTR svrEventCb;
    GetAudioCb              svrAudioCb;
    GetVideoCb              svrVideoCb;
} SVR_Ops;


int SVR_Init(const SVR_Ops *ops);
int SVR_Deinit(void);

int SVR_StartGetAudio(void);
int SVR_StopGetAudio(void);

int SVR_StartGetVideo(void);
int SVR_StopGetVideo(void);

int SVR_StartRecvAudio(void);
int SVR_SndTaklAudio (char *data, int len, unsigned long long pts, int encode, int sampleRate);
int SVR_StopRecvAudio(void);

int SVR_Login(bool bLogin);
int SVR_RequestFileDir(void);
int SVR_RequestGetDevInfo(void);
int SVR_RequestStartRec(void);
int SVR_RequestStopRec(void);
int SVR_RequestTakePhoto(void);

#ifdef __cplusplus
}
#endif