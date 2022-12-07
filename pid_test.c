#include <stdio.h>
#include <time.h>
#include "my_pid.h"

int main(void)
{
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

    return 0;
}