#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

typedef enum av_H264E_TYPE_E {
    H264E_BSLICE = 0,                         /* B SLICE types */
    H264E_PSLICE = 1,                         /* P SLICE types */
    H264E_ISLICE = 2,                         /* I SLICE types */
    H264E_IDRSLICE = 5,                       /* IDR SLICE types */
    H264E_SEI    = 6,                         /* SEI types */
    H264E_SPS    = 7,                         /* SPS types */
    H264E_PPS    = 8,                         /* PPS types */
    H264E_BUTT
} AV_H264E_TYPE_E;

typedef enum av_H265E_TYPE_E {
    H265E_BSLICE = 0,                          /* B SLICE types */
    H265E_PSLICE = 1,                          /* P SLICE types */
    H265E_ISLICE = 2,                          /* I SLICE types */
    H265E_IDRSLICE = 19,                       /* IDR SLICE types */
    H265E_VPS    = 32,                         /* VPS types */
    H265E_SPS    = 33,                         /* SPS types */
    H265E_PPS    = 34,                         /* PPS types */
    H265E_SEI    = 39,                         /* SEI types */
    H265E_BUTT
} AV_H265E_TYPE_E;

#define VIDEO_ENCODE_H264          0x0
#define VIDEO_ENCODE_H265          0x1

#define AUDIO_ENCODE_PCM           0x0 /* only support pcm and 8k sample */
#define AUDIO_ENCODE_AAC_LC        0x1
#define AUDIO_ENCODE_PCMA          0x2

#define EVENT_PAYLOAD_LEN          (512)

typedef enum SVR_EVENTID {
    EVENT_GET_VIDEO = 0X20000,
    EVENT_GET_AUDIO,
    EVENT_SND_TALK_AUDIO,
    EVENT_CTL_START_REC,
    EVENT_CTL_TAKE_PHOTO,
    EVENT_CTL_GET_DIR,
    EVENT_CTL_GET_DEV_INFO,
    EVENT_CTL_SOS,
    EVENT_CTL_PTT,
    EVENT_LOGIN_STATUS,
    EVENT_BUTT
}SVR_EVENTID;

typedef struct event {
    unsigned int eventID;
    int          argv1;
    int          result;
    time_t       createTime;
    char         aszPayload[EVENT_PAYLOAD_LEN];
} EVENT;

typedef int (*EVTHUB_EVENTPROC_FN_PTR)(EVENT *pEvent);
