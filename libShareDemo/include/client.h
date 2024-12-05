#pragma once

#include "shareType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*GetTalkCb) (const char *data, int len, unsigned long long pts, int encode, int sampleRate);
typedef struct {
    EVTHUB_EVENTPROC_FN_PTR cliEventCb;
    GetTalkCb              cliTalkCb;
} CLI_Ops;

int CLI_Init(const CLI_Ops *ops);
int CLI_Deinit();

int CLI_SndAudio (char *data, int len, unsigned long long pts, int encode, int sampleRate);
int CLI_SndVIDEO (char *data, int len, unsigned long long pts, int encode, int frameType);
int CLI_StartProcTalk(void);
int CLI_StopProcTalk(void);
int CLI_SndMsg(EVENT *event);

#ifdef __cplusplus
}
#endif