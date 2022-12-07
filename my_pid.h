#ifndef _MY_PID_H_
#define _MY_PID_H_

/* INCLUDES FILE IN HERE */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* 温控过程中，是否将温控的方向进行区分，比如分为升温和降温两个过程 */
#define PID_ENABLE_CTRL_DIR_SPLIT   1
/* 温控过程中，是否将温控的过程分为多个步骤，比如分为最大功率控制和稳定控制两个步骤 */
#define PID_ENABLE_PROCESS_SPLIT    1
/* 需要进行温控的通道数量 */
#define PID_CH_NUM      2
/* 默认的目标温度，单位为摄氏度 */
#define PID_DEFAULT_TARGET_TEMP     25
/* 默认的PID时间间隔，单位为秒 */
#define PID_DEFAULT_DELTA_TIME      1
/* 获取通道ch的当前温度接口定义 */
#define GET_CUR_TEMP(ch)    (50)
/* PID控制过程中的一些log信息输出接口定义 */
#define DBG_INFO(fmt, ...)      printf(fmt, ##__VA_ARGS__)
#define DBG_ERROR(fmt, ...)     printf(fmt, ##__VA_ARGS__)

typedef enum _PID_CTRL_STA {
    PID_CTRL_STA_DISABLE = 0,       /* 失能PID控制，默认*/
    PID_CTRL_STA_ENABLE             /* 使能PID控制*/
} PidCtrlStateDef;

#if PID_ENABLE_PROCESS_SPLIT
    /* 将温控过程分为快速升降温阶段和温度阶段，使得更快速温度得达到目标温度 */
    typedef enum _PID_CTRL_PROC {
        PID_CTRL_PROC_FAST = 0,         /* 快速温变阶段 */
        PID_CTRL_PROC_STABLE,           /* 稳定阶段 */
        PID_CTRL_PROC_NUM
    } PidCtrlProcDef;
#endif /* PID_ENABLE_PROCESS_SPLIT */

#if PID_ENABLE_CTRL_DIR_SPLIT 
    /* 温控过程中，升温和降温的PID参数可能是不同的 */
    typedef enum _PID_CTRL_DIR {
        PID_CTRL_DIR_HEAT = 0,          /* 升温阶段 */
        PID_CTRL_DIR_COLD,              /* 降温阶段 */
        PID_CTRL_DIR_NUM
    } PidCtrlDirDef;
#endif /* PID_ENABLE_CTRL_DIR_SPLIT */

typedef struct _PID_PARAM {
    float kp;
    float ki;
    float kd;
} PidParamDef;

typedef struct {
    float tarTemp;                  /* 目标温度, Unit[Degree]*/
    float deltaT;                   /* PID 温控的间隔时间, Unit[Second] */ 
    float lastErr;                  /* 上次温差 */
    float integral;                 /* 积分累积项 */
    PidCtrlStateDef pidCtrlSta;     /* PID 是否使能 */

    #if PID_ENABLE_PROCESS_SPLIT
        PidCtrlProcDef pidCtrlProc;
        #if PID_ENABLE_CTRL_DIR_SPLIT 
            PidCtrlDirDef pidCtrlDir;                                   /* 控温方向 */
            float diffTemp[PID_CTRL_DIR_NUM];                           /* 当前温度和目标温度的温差小于diffTemp时，PidCtrlProcDef的状态进行转换 */
            PidParamDef pidParam[PID_CTRL_DIR_NUM][PID_CTRL_PROC_NUM];  /* PID 参数 */
        #else
            float diffTemp;
            PidParamDef PidParam[PID_CTRL_PROC_NUM];  
        #endif /* PID_ENABLE_CTRL_DIR_SPLIT */
    #else
        #if PID_ENABLE_CTRL_DIR_SPLIT 
            PidCtrlDirDef pidCtrlDir; 
            PidParamDef pidParam[PID_CTRL_DIR_NUM];
        #else
            PidParamDef pidParam;
        #endif /* PID_ENABLE_CTRL_DIR_SPLIT */
    #endif /* PID_ENABLE_PROCESS_SPLIT */
} TempCtrlDef;

extern TempCtrlDef tempCtrl[PID_CH_NUM];

void PID_Init(void);
void PID(uint8_t ch, TempCtrlDef* pTempCtrl);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _MY_PID_H_*/
