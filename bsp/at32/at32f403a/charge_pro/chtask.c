/*******************************************************************************
 * @file
 * @note
 * @brief
 *
 * @author
 * @date     2021-05-02
 * @version  V1.0.0
 *
 * @Description
 *
 * @note History:
 * @note     <author>   <time>    <version >   <desc>
 * @note
 * @warning
 *******************************************************************************/
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "chtask.h"
#include "ch_port.h"
#include "rn8209d.h"
#include "common.h"
#include "w25qxx.h"
#include "dwin_com_pro.h"
#include "4GMain.h"
#define CH_THREAD_STK_SIZE     2048
#define CH_THREAD_PRIO         RT_THREAD_PRIORITY_MAX-10
#define CH_THREAD_TIMESLICE    10
#define CH_MSG_MAX_SIZE        256

rt_thread_t pstChThread = RT_NULL;
CH_TASK_T stChTcb = {0};
CH_TASK_T *pstCh;

/* 消息队列中用到的放置消息的内存池 */
uint8_t ucaMsgBuf[CH_MSG_MAX_SIZE] = {0};


/**
 * @brief 充电任务线程数据初始化
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_task_init(void)
{
    mq_service_bind(CM_CHTASK_MODULE_ID,"ch task mq");

    cp_adc_init();

    ch_ctl_init();

    cp_pwm_init();

//	ch_rate_init();
//	ch_ctl_enable();
//	rt_thread_mdelay(300);

    ch_ctl_disable();
    rt_thread_mdelay(200);
    Init_HLW8112();		//计量芯片初始化
    if(DisSysConfigInfo.energymeter == 1)
    {
        input_io_init(&stChTcb.stRn8209);
    }
    else
    {
        input_io_init(&stChTcb.stCh);
    }
    spiflash_init();
    return;
}

/**
 * @brief 获取cp状态信息(12v,9v,6v)
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_get_cp_state_info(void)
{
    stChTcb.stGunStat.PWMcycle  = ch_get_cp_volt(&stChTcb.stGunStat.uiCpVolt);  //不取平均值

    if((CP_12V_MAX >= stChTcb.stGunStat.uiCpVolt) && (CP_12V_MIN  <= stChTcb.stGunStat.uiCpVolt))          /* 在12V 范围 */
    {
        stChTcb.stGunStat.ucCpStat = CP_12V;
        return;
    }

    if((CP_9V_MAX >= stChTcb.stGunStat.uiCpVolt) && (CP_9V_MIN <= stChTcb.stGunStat.uiCpVolt))              /* 在9V 范围 */
    {
        stChTcb.stGunStat.ucCpStat = CP_9V;
        return;
    }

    if((CP_6V_MAX >= stChTcb.stGunStat.uiCpVolt) && (CP_6V_MIN <= stChTcb.stGunStat.uiCpVolt))             /* 在6V 范围 */
    {
        stChTcb.stGunStat.ucCpStat = CP_6V;
        return;
    }

    stChTcb.stGunStat.ucCpStat = CP_ABNORMAL;                    /* 异常 */
}

extern SYSTEM_RTCTIME gs_SysTime;
/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static uint8_t ch_from_ctl_msg(CH_CTL_T *pstChCtl)
{
    if(CH_START_CTL == pstChCtl->ucCtl)                                             /* 充电启动 */
    {
        if(stChTcb.ucState == CHARGING)
        {
            return 0;
        }
        if(stChTcb.ucState == INSERT)                                               /* 在插枪状态可以启动 */
        {
            stChTcb.stChCtl.ucCtl = CH_START_CTL;
            stChTcb.stChCtl.ucChStartMode = pstChCtl->ucChStartMode;
            stChTcb.stChCtl.ucChMode = pstChCtl->ucChMode;
            stChTcb.stChCtl.uiStopParam = pstChCtl->uiStopParam;

        }
        else
        {
            memset(&stChTcb.stCh,0,sizeof(CH_T));
            stChTcb.stCh.uiChStartTime = gs_SysTime;
            stChTcb.stCh.timestart = time(RT_NULL);
            mq_service_ch_send_dip(CH_TO_DIP_STARTFAIL ,UNPLUG ,0,NULL);  //枪未插

        }
    }
    else
    {
        if((CH_START_CTL == stChTcb.stChCtl.ucCtl) )
        {
            stChTcb.stChCtl.ucCtl        = CH_STOP_CTL;
        }
    }
    //mq_service_ch_send_msg_to_tcu(CH_TO_TCU_CHCTL_ACK,0,sizeof(CHCTL_ACK_T),(uint8_t *)&stChCtlAckData);
    return 1;

}


/**
 * @brief 处理tcu消息
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_task_tcu_msg_proc(MQ_MSG_T *pstMsg)
{
    switch(pstMsg->uiMsgCode)
    {
    case TCU_TO_CH_CHCLT:
        ch_from_ctl_msg((CH_CTL_T *)pstMsg->pucLoad);
        break;
    default:
        break;
    }
}


/**
 * @brief 消息处理
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void ch_msg_proc(MQ_MSG_T *pstMsg)
{
    switch(pstMsg->uiSrcMoudleId)
    {
        /* 目前TCU 模块和刷卡模块只向充电任务功能模块发送了充电控制消息 */
    case CM_TCUTASK_MODULE_ID:
        rt_kprintf("msg src module id:%d, pstMsg->uiMsgCode = %d\n\r",pstMsg->uiSrcMoudleId,pstMsg->uiMsgCode);
        ch_task_tcu_msg_proc(pstMsg);
        break;
    default:
        rt_kprintf("msg src module id:%d\n\r",pstMsg->uiSrcMoudleId);
        break;
    }
}

/**
 * @brief 充电任务等待消息
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void ch_wait_msg(void)
{
    MQ_MSG_T stMsg = {0};

    if(mq_service_recv_msg(CM_CHTASK_MODULE_ID,&stMsg,ucaMsgBuf,sizeof(ucaMsgBuf),CM_TIME_20_MSEC) == 0)
    {
        ch_msg_proc(&stMsg);
    }
}


/**
 * @brief 充电任务线程入口
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void HLW8112DataDispose(uint8_t num)
{
    static float kwh = 0;
    static uint32_t nowSysTime = 0,lastSysTime = 0;

    //uint32_t Vol_A = 0,Cur_A = 0,Vol_B = 0,Cur_B = 0,Vol_C = 0,Cur_C = 0;
    if(num == HLW8110_A)
    {
        stChTcb.stRn8209.usVolt = AF_AC_V * 10;      //A相电压
        stChTcb.stRn8209.usCurrent = AF_AC_I * 100;   //A相电流
        stChTcb.stRn8209.uiE_A = AF_AC_E * 1000;    //A相总电能
    }

    if(num == HLW8110_B)
    {
        stChTcb.stRn8209.usVoltb = BF_AC_V * 10;        //B相电压
        stChTcb.stRn8209.usCurrentb = BF_AC_I * 100;   //B相电流
        stChTcb.stRn8209.uiE_B = BF_AC_E * 1000;      //B相总电能
    }

    if(num == HLW8110_C)
    {
        stChTcb.stRn8209.usVoltc = CF_AC_V * 10;      //C相电压
        stChTcb.stRn8209.usCurrentc = CF_AC_I * 100;   //C相电流
        stChTcb.stRn8209.uiE_C = CF_AC_E * 1000;      //C相总电能
    }
//    stChTcb.stRn8209.uiE = stChTcb.stRn8209.uiE_A + stChTcb.stRn8209.uiE_B + stChTcb.stRn8209.uiE_C;   //ABC三相总和电能

    stChTcb.stRn8209.uiE = stChTcb.stCh.viDChQ[0] + stChTcb.stCh.viDChQ[1]+ stChTcb.stCh.viDChQ[2];   //ABC三相总和电能---用定时器自我计算2024年1月17日

    nowSysTime = rt_tick_get();
    //3个电表其中有一个电流大于1A，持续1分钟没有走电量
    if((stChTcb.stRn8209.usCurrent > 100) || (stChTcb.stRn8209.usCurrentb > 100) ||(stChTcb.stRn8209.usCurrentc > 100))
    {
        if(kwh == stChTcb.stRn8209.uiE)
        {
            if((nowSysTime >= lastSysTime) ? ((nowSysTime - lastSysTime) >= (CM_TIME_60_SEC)):((nowSysTime + (0xffffffff - lastSysTime)) >= (CM_TIME_60_SEC)) )
            {
                Init_HLW8112();  //初始化芯片
            }
        }
        else
        {
            lastSysTime = nowSysTime;
            kwh = stChTcb.stRn8209.uiE;
        }
    }
    else
    {
        lastSysTime = nowSysTime;
    }
}


/**
 * @brief 周期性获取rn8209信息
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_cycle_get_8209(void)
{
    static uint8_t i = HLW8110_A;
    uint32_t uiTick = 0;
    uiTick = rt_tick_get();
    if(uiTick - stChTcb.stRn8209.uiGetTime > CH_TIME_50_MSEC)
    {
        stChTcb.stRn8209.uiGetTime = uiTick;
        HLW8112_Measure(); //依次读取3个电表的A通道   V/A/电量信息
        HLW8112DataDispose(i);  //各项赋值
        i++;
        if(i >= HLW8110_MAX)
        {
            i = HLW8110_A;
        }
    }
}





/**
 * @brief
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
void ch_task_cycle_proc(void)
{
    if(DisSysConfigInfo.energymeter == 1)
    {
        ch_cycle_get_8209();
    }
}
/**
 * @brief 充电任务线程入口
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static void ch_thread_entry(void* parg)
{
    ch_task_init();
    while(1)
    {
        if(APP_GetSIM7600Mode() == MODE_HTTP)
        {
            rt_thread_mdelay(1000); //远程升级其他无关数据帧都不不要发送和处理
            continue;
        }
        ch_wait_msg();
        ch_get_cp_state_info();   	//CP电压检测
        ch_task_cycle_proc() ; 		//计量信息读取
        ch_loop_proc(&stChTcb);
        rt_thread_mdelay(1);
    }
}

/**
 * @brief 充电任务线程初始化
 * @param[in]
 * @param[out]
 * @return
 * @note
 */
static int ch_thread_init(void)
{
    pstChThread = rt_thread_create("CH_Thread",
                                   ch_thread_entry,
                                   RT_NULL,
                                   CH_THREAD_STK_SIZE,
                                   CH_THREAD_PRIO,
                                   CH_THREAD_TIMESLICE);

    if(RT_NULL != pstChThread)
    {
        rt_kprintf("The ch thread create suceess\r\n");
        rt_thread_startup(pstChThread);
    }
    else
    {
        goto __exit;
    }
    return RT_EOK;

__exit:
    if (pstChThread)
    {
        rt_thread_delete(pstChThread);
    }
    return RT_ERROR;
}
INIT_APP_EXPORT(ch_thread_init);
