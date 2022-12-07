
# PID by C

## 介绍

这是一个通过C语言实现的PID算法。
但是添加了一些常用的额外配置项：比如一个设备升温和降温差异很大，升降温不能用同一组 `PID` 参数实现温控；
又比如用户想要在当前温度距离目标温度还有较大差距时直接全功率输出，直到差距小于某个阈值后才加入 `PID` 温控，以实现更快速地温控；

## 注意

[1] 在 `pid_test.c` 文件中调用了，`time.h` 中的 `_sleep(1000);` 函数进行延时，该函数只在 `windows` 系统下有效，
如果想要在其他系统运行，需要自己实现这个延时函数。

## 使用方式

[1] 根据自己的需要，设置 `my_pid.h` 中的以下变量：

```C
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
```

[2] 根据实际情况，初始化 `my_pid.c` 中的以下变量：

```C
#if PID_ENABLE_PROCESS_SPLIT
    #if PID_ENABLE_CTRL_DIR_SPLIT
        const float DIFF_TEMP[PID_CTRL_DIR_NUM] = {5, 5};
        const PidParamDef PID_PARAM[PID_CTRL_DIR_NUM][PID_CTRL_PROC_NUM] = {    /* FAST       STABLE   */
                                                                                {{1, 2, 3}, {4, 5, 6}},     /* HEAT */
                                                                                {{7, 8, 9}, {10, 11, 12}}   /* COLD */
                                                                            };
    #else
        const float DIFF_TEMP = 5;
        const PidParamDef PID_PARAM[PID_CTRL_PROC_NUM] = {{1, 5, 3}, {4, 2, 6}};  /* FAST, STABLE */
    #endif /* PID_ENABLE_CTRL_DIR_SPLIT */
#else
    #if PID_ENABLE_CTRL_DIR_SPLIT
        const PidParamDef PID_PARAM[PID_CTRL_DIR_NUM] = {{1, 2, 3}, {4, 5, 6}}; /* HEAT, COLD */
    #else
        const PidParamDef PID_PARAM = {1, 2, 3};
    #endif /* PID_ENABLE_CTRL_DIR_SPLIT */
#endif /* PID_ENABLE_PROCESS_SPLIT */
```

[3] 按如下顺序实现PID控制：

```C
/* 1. Init the PID params */
PID_Init();

/* 2. Set tarTemp */ 
for(uint8_t i = 0; i < PID_CH_NUM; i++) {
    tempCtrl[i].tarTemp = 60;
}

/* 3. According the tarTemp to decide the pidCtrlDir, if have the pidCtrlDir control */
#if PID_ENABLE_CTRL_DIR_SPLIT
    for(uint8_t i = 0; i < PID_CH_NUM; i++) {
        if(GET_CUR_TEMP(i) < tempCtrl[i].tarTemp) {
            tempCtrl[i].pidCtrlDir = PID_CTRL_DIR_HEAT;
        } else {
            tempCtrl[i].pidCtrlDir = PID_CTRL_DIR_COLD;
        }
    }
#endif /* PID_ENABLE_CTRL_DIR_SPLIT */

/* 4. Enable the PID control */
for(uint8_t i = 0; i < PID_CH_NUM; i++) {
    tempCtrl[i].pidCtrlSta = PID_CTRL_STA_ENABLE;
} 

/* 5. start PID control */
while(1) {
    for(uint8_t i = 0; i < PID_CH_NUM; i++) {
        PID(i, &tempCtrl[i]);
    }
    printf("\r\n");
    _sleep(1000);       /* delay in ms */
}
```

## 参考

[WIKI: PID controller](https://en.wikipedia.org/wiki/PID_controller)