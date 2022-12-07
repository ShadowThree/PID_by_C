#include "my_pid.h"

TempCtrlDef tempCtrl[PID_CH_NUM];

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

void PID_Init(void)
{
    for(uint8_t i = 0; i < PID_CH_NUM; i++) {
        tempCtrl[i].tarTemp = PID_DEFAULT_TARGET_TEMP;
        tempCtrl[i].deltaT = PID_DEFAULT_DELTA_TIME;
        tempCtrl[i].lastErr = 0;
        tempCtrl[i].integral = 0;
        tempCtrl[i].pidCtrlSta = PID_CTRL_STA_DISABLE;
        #if PID_ENABLE_PROCESS_SPLIT
            tempCtrl[i].pidCtrlProc = PID_CTRL_PROC_FAST;
            #if PID_ENABLE_CTRL_DIR_SPLIT
                tempCtrl[i].pidCtrlDir = PID_CTRL_DIR_HEAT;
                for(uint8_t j = 0; j < PID_CTRL_DIR_NUM; j++) {
                    tempCtrl[i].diffTemp[j] = DIFF_TEMP[j];
                    for(uint8_t k = 0; k < PID_CTRL_PROC_NUM; k++) {
                        tempCtrl[i].pidParam[j][k].kp = PID_PARAM[j][k].kp;
                        tempCtrl[i].pidParam[j][k].ki = PID_PARAM[j][k].ki;
                        tempCtrl[i].pidParam[j][k].kd = PID_PARAM[j][k].kd;
                    }
                }
            #else
                tempCtrl[i].diffTemp = DIFF_TEMP;
                for(uint8_t j = 0; j < PID_CTRL_PROC_NUM; j++) {
                    tempCtrl[i].PidParam[j].kp = PID_PARAM[j].kp;
                    tempCtrl[i].PidParam[j].ki = PID_PARAM[j].ki;
                    tempCtrl[i].PidParam[j].kd = PID_PARAM[j].kd;
                }
            #endif /* PID_ENABLE_CTRL_DIR_SPLIT */
        #else
            #if PID_ENABLE_CTRL_DIR_SPLIT
                tempCtrl[i].pidCtrlDir = PID_CTRL_DIR_HEAT; 
                for(uint8_t j = 0; j < PID_CTRL_DIR_NUM; j++) {
                    tempCtrl[i].pidParam[j].kp = PID_PARAM[j].kp;
                    tempCtrl[i].pidParam[j].ki = PID_PARAM[j].ki;
                    tempCtrl[i].pidParam[j].kd = PID_PARAM[j].kd;
                }
            #else
                tempCtrl[i].pidParam.kp = PID_PARAM.kp;
                tempCtrl[i].pidParam.ki = PID_PARAM.ki;
                tempCtrl[i].pidParam.kd = PID_PARAM.kd;
            #endif /* PID_ENABLE_CTRL_DIR_SPLIT */
        #endif /* PID_ENABLE_PROCESS_SPLIT */
    }
}

void PID(uint8_t ch, TempCtrlDef* pTempCtrl)
{
    float curTemp;
    float err;
    float output;
    PidParamDef* pidParam;

    if(ch >= PID_CH_NUM) {
        DBG_ERROR("ERR: channel number out of range!\r\n");
        return;
    }

    if(pTempCtrl == NULL) {
        DBG_ERROR("ERR: pTempCtrl is NULL!\r\n");
        return;
    }

    if(pTempCtrl->pidCtrlSta == PID_CTRL_STA_ENABLE) {
        curTemp = GET_CUR_TEMP(ch);
        err = pTempCtrl->tarTemp - curTemp;

        #if PID_ENABLE_PROCESS_SPLIT
            #if PID_ENABLE_CTRL_DIR_SPLIT
                if((pTempCtrl->pidCtrlProc != PID_CTRL_PROC_STABLE) && (((err<0)?(-err):(err)) < pTempCtrl->diffTemp[pTempCtrl->pidCtrlDir])) {
                    pTempCtrl->pidCtrlProc = PID_CTRL_PROC_STABLE;
                    pTempCtrl->integral = 0;
                    DBG_INFO("CH%d curT[%f] tarT[%f] DIR[%d] FAST-->STABLE\r\n", ch, curTemp, pTempCtrl->tarTemp, pTempCtrl->pidCtrlDir);
                }
                pidParam = &pTempCtrl->pidParam[pTempCtrl->pidCtrlDir][pTempCtrl->pidCtrlProc];
            #else
                if((pTempCtrl->pidCtrlProc != PID_CTRL_PROC_STABLE) && (((err<0)?(-err):(err)) < pTempCtrl->diffTemp)) {
                    pTempCtrl->pidCtrlProc = PID_CTRL_PROC_STABLE;
                    pTempCtrl->integral = 0;
                    DBG_INFO("CH%d curT[%f] tarT[%f] FAST-->STABLE\r\n", ch, curTemp, pTempCtrl->tarTemp);
                }
                pidParam = &pTempCtrl->PidParam[pTempCtrl->pidCtrlProc];
            #endif /* PID_ENABLE_CTRL_DIR_SPLIT */ 
        #else
            #if PID_ENABLE_CTRL_DIR_SPLIT
                pidParam = &pTempCtrl->pidParam[pTempCtrl->pidCtrlDir];
            #else
                pidParam = &pTempCtrl->pidParam;
            #endif /* PID_ENABLE_CTRL_DIR_SPLIT */ 
        #endif /* PID_ENABLE_PROCESS_SPLIT */

        pTempCtrl->integral += err * pTempCtrl->deltaT;
        output = pidParam->kp * err + pidParam->ki * pTempCtrl->integral + pidParam->kd * (err-pTempCtrl->lastErr) / pTempCtrl->deltaT;
        pTempCtrl->lastErr = err;

        #if PID_ENABLE_PROCESS_SPLIT
            #if PID_ENABLE_CTRL_DIR_SPLIT 
                DBG_INFO("CH%d Dir[%d] Proc[%d] curT[%f] tarT[%f] P[%f] I[%f] D[%f] out[%f]\r\n",
                    ch, pTempCtrl->pidCtrlDir, pTempCtrl->pidCtrlProc, curTemp, pTempCtrl->tarTemp,
                    pidParam->kp, pidParam->ki, pidParam->kd, output);
            #else
                DBG_INFO("CH%d Proc[%d] curT[%f] tarT[%f] P[%f] I[%f] D[%f] out[%f]\r\n",
                    ch, pTempCtrl->pidCtrlProc, curTemp, pTempCtrl->tarTemp,
                    pidParam->kp, pidParam->ki, pidParam->kd, output); 
            #endif /* PID_ENABLE_CTRL_DIR_SPLIT */ 
        #else
            #if PID_ENABLE_CTRL_DIR_SPLIT
                DBG_INFO("CH%d Dir[%d] curT[%f] tarT[%f] P[%f] I[%f] D[%f] out[%f]\r\n",
                    ch, pTempCtrl->pidCtrlDir, curTemp, pTempCtrl->tarTemp,
                    pidParam->kp, pidParam->ki, pidParam->kd, output);
            #else
                DBG_INFO("CH%d curT[%f] tarT[%f] P[%f] I[%f] D[%f] out[%f]\r\n",
                    ch, curTemp, pTempCtrl->tarTemp, pidParam->kp, pidParam->ki, pidParam->kd, output);
            #endif /* PID_ENABLE_CTRL_DIR_SPLIT */
        #endif /* PID_ENABLE_PROCESS_SPLIT */
    }
}
