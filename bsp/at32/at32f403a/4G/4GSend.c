/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSMain.c
* Author			:
* Date First Issued	:
* Version			:
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "4GMain.h"
#include "4GRecv.h"
#include <string.h>
#include "APFrame.h"
#include "DwinProtocol.h"
#include "dwin_com_pro.h"
#include "fal_cfg.h"

/* Private define-----------------------------------------------------------------------------*/
#define   GPRSSEND_Q_LEN  								20
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/


uint8_t OFFlineBuf[300];		//离线交易记录写入读取换成
uint8_t OFFFSlineBuf[300];	//离线分时交易记录写入读取换成

/*****************************************************************************
* Function     : Task4GSend
* Description  : 4G发送任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日
*****************************************************************************/
void Task4GSend(void *pdata)
{
    MQ_MSG_T stMsg = {0};
    static uint8_t sendbuf[50];
    uint8_t* pBillGunA = APP_GetBillInfo(GUN_A);
    uint8_t* pBillGunB = APP_GetBillInfo(GUN_B);
    uint8_t i = 0;
    static uint32_t curtime = 0,lasttime = 0;
    rt_thread_mdelay(2000);
    if(DisSysConfigInfo.standaloneornet != DISP_NET)
    {
        return;
    }
    mq_service_bind(CM_4GSEND_MODULE_ID,"4G send mq");
    while(1)
    {
        if(mq_service_recv_msg(CM_4GSEND_MODULE_ID,&stMsg,sendbuf,sizeof(sendbuf),CM_TIME_1_SEC) == 0 )
        {
            if(APP_GetSIM7600Mode() == MODE_HTTP)   //远程升级其他无关数据帧都不不要发送和处理
            {
                continue;
            }
            switch(stMsg.uiMsgCode)   //消息码
            {
            case APP_START_ACK:		//开始充电应答
                if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
                {
                    if(stMsg.uiMsgVar < GUN_MAX) //枪号保护
                    {
                        _4G_SendStartAck((_GUN_NUM)stMsg.uiMsgVar);
                        if(NetConfigInfo[NET_YX_SELCT].NetNum > 1)
                        {
                            ZF_SendStartCharge();				//发送启动帧
                        }
                    }
                }
                break;

            case APP_STOP_ACK:		//停止充电应答
                if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
                {
                    if(stMsg.uiMsgVar < GUN_MAX) //枪号保护
                    {
                        _4G_SendStopAck((_GUN_NUM)stMsg.uiMsgVar);
                        //HFQG_SendBill((_GUN_NUM)pMsg->DataLen);    //发送结算
                    }
                }
                break;
            case APP_STOP_BILL:		//停止结算
                //if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
                //{

                if(stMsg.uiMsgVar < GUN_MAX) //枪号保护
                {
#if(NET_YX_SELCT == XY_AP)
                    {
                        if(APP_GetStartNetState(stMsg.uiMsgVar) == NET_STATE_ONLINE)
                        {
                            //在线记录
                            WriterFmBill(stMsg.uiMsgVar,1);
                            APP_SetResendBillState(stMsg.uiMsgVar,RT_TRUE);
                            _4G_SendBill((_GUN_NUM)stMsg.uiMsgVar);
                        }
                        else
                        {
                            //不管A,B枪的离线交易记录都用一个，统一用A
                            ResendBillControl[GUN_A].OffLineNum = APP_GetNetOFFLineRecodeNum();
                            if(ResendBillControl[GUN_A].OffLineNum < 100)
                            {
                                ResendBillControl[GUN_A].OffLineNum++;			//离线交易记录+1
                                APP_SetNetOFFLineRecodeNum(ResendBillControl[GUN_A].OffLineNum);
                                PreAPOffLineBill(stMsg.uiMsgVar,OFFlineBuf); 		//读取离线交易记录
                                PreAPFSOffLineBill(stMsg.uiMsgVar,OFFFSlineBuf);		//离线分时交易记录
                                APP_RWNetOFFLineRecode(ResendBillControl[GUN_A].OffLineNum,FLASH_ORDER_WRITE,OFFlineBuf);
                                APP_RWNetFSOFFLineRecode(ResendBillControl[GUN_A].OffLineNum,FLASH_ORDER_WRITE,OFFFSlineBuf);
                            }
                        }
                    }
#else
                    {
                        //OSTimeDly(SYS_DELAY_3s);
                        WriterFmBill(stMsg.uiMsgVar,1);
                        APP_SetResendBillState(stMsg.uiMsgVar,RT_TRUE);
                        _4G_SendBill((_GUN_NUM)stMsg.uiMsgVar);
                        _4G_SetStartType((_GUN_NUM)stMsg.uiMsgVar,_4G_APP_START);			//设置为APP
                    }
#endif
                }
                //}
                if(NetConfigInfo[NET_YX_SELCT].NetNum > 1)
                {
                    ZF_SendBill();
                }
                break;
            case	APP_STE_DEVS:     //查询设备状态
                _4G_SendDevState(stMsg.uiMsgVar);			//发送设备状态
                break;
            case	APP_UPDADA_BALANCE:     //更新余额

#if(NET_YX_SELCT == XY_YKC)
            {
                YKC_SendBalanceAck(stMsg.uiMsgVar);			//更新余额
            }
#endif
            break;
#if(NET_YX_SELCT == XY_YKC)
            case	APP_SJDATA_QUERY:     //读取实时数据
                if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
                {
                    if(stMsg.uiMsgVar == GUN_A)
                    {
                        YKC_SendSJDataGunACmd13();
                    }
                    if(stMsg.uiMsgVar == GUN_B)
                    {
                        YKC_SendSJDataGunBCmd13();
                    }
                }
                break;
#endif
            case	APP_STE_BILL:     //查询订单
                if(stMsg.uiMsgVar == GUN_A)
                {
                    ReSendBill(GUN_A,pBillGunA,1);
                }
                else
                {
                    ReSendBill(GUN_B,pBillGunB,1);
                }
                break;
            case APP_RATE_ACK:		//停止结算
                _4G_SendRateAck(stMsg.uiMsgVar);	 //费率设置应答
                break;
            case APP_QUERY_RATE:	//费率请求
                _4G_SendQueryRate();
                break;
            case APP_RATE_MODE:		//计费模型验证
                _4G_SendRateMode();
                break;
            case APP_STE_RATE:			//费率应答
                HY_SendQueryRateAck();
                break;
            case APP_STE_TIME:			//校准时间
                _4G_SendSetTimeAck();
                break;

            case APP_CARDVIN_CHARGE:
                _4G_SendCardVinCharge(stMsg.uiMsgVar);   //刷卡启动上报
                break;
            case APP_CARD_INFO:						//卡鉴权
                _4G_SendCardInfo(stMsg.uiMsgVar);
                break;

            case APP_VIN_INFO:						//Vin鉴权
                _4G_SendVinInfo(stMsg.uiMsgVar);
                break;
            case APP_CARD_WL:			//卡白名单
#if(NET_YX_SELCT == XY_AP)
            {
                AP_SendCardWLAck();
            }
#endif
            break;
            case APP_VIN_WL:			//·	VIN白名单
#if(NET_YX_SELCT == XY_AP)
            {
                AP_SendVinWLAck();
            }
#endif
            break;
            case APP_VINCARD_RES:
#if(NET_YX_SELCT == XY_AP)
            {
                AP_SendVinCardResAck();
            }
#endif
            break;
            case APP_OFFLINE_ACK:
#if(NET_YX_SELCT == XY_AP)
            {
                if(ResendBillControl[GUN_A].OffLineNum)
                {
                    //离线记录应答了 -1
                    ResendBillControl[GUN_A].OffLineNum--;
                    APP_SetNetOFFLineRecodeNum(ResendBillControl[GUN_A].OffLineNum);
                }
            }
#endif
            break;
            case BSP_4G_SENDNET1:
                if(NetConfigInfo[NET_YX_SELCT].NetNum > 1)
                {
                    Send_AT_CIPRXGET(0);   //主动读取数据
                }
                break;
            case BSP_4G_SENDNET2:
                if(NetConfigInfo[NET_YX_SELCT].NetNum > 1)
                {
                    Send_AT_CIPRXGET(1);	//主动读取数据
                }
                break;
            default:
                break;

            }
        }
        if(APP_GetSIM7600Mode() == MODE_HTTP)   //远程升级其他无关数据帧都不不要发送和处理
        {
            continue;
        }
        for(i = 0; i < NetConfigInfo[NET_YX_SELCT].NetNum; i++)
        {
            if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(i) == STATE_OK)) //连接上服务器
            {
                //发送数据给服务器
                //ModuleSIM7600_SendData(i, (uint8_t*)"hello word qiangge\r\n", strlen("hello word qiangge\r\n"));
                if(i == 0)
                {
                    _4G_SendFrameDispose();  //合肥乾古周期性发送帧
                }
                else
                {
                    ZF_SendFrameDispose();
                }
            }
        }
        if (NetConfigInfo[NET_YX_SELCT].NetNum > 1)
        {
            //每10s读取一次信号强度
            if((curtime >= lasttime) ? ((curtime - lasttime) >= CM_TIME_10_SEC) : \
                    ((curtime + (0xffffffffU - lasttime)) >= CM_TIME_10_SEC))
            {
                lasttime = curtime;
                Send_AT_CSQ();				//读取信号强度
            }
        }
        //处理发送订单
        for(i = 0; i < GUN_MAX; i++)
        {
            if(APP_GetResendBillState(i) == RT_TRUE)
            {
                if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK) && (APP_GetAppRegisterState(0) == STATE_OK)	) //连接上服务器
                {
                    if(i == GUN_A)
                    {
                        ReSendBill(i,pBillGunA,0);
                    }
                    else
                    {
                        //	WriterFmBill(1,1);
                        ReSendBill(i,pBillGunB,0);
                    }
                }
            }
            else
            {
                ResendBillControl[i].CurTime = rt_tick_get();		//获取当前时间
                ResendBillControl[i].LastTime = ResendBillControl[i].CurTime;
            }
            if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(i) == STATE_OK) && (APP_GetAppRegisterState(i) == STATE_OK)	) //连接上服务器
            {
#if(NET_YX_SELCT == XY_AP)
                {
                    ReSendOffLineBill();      //离线交易记录获取
                }
#endif
            }


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
static int _4G_send_init(void)
{

    rt_thread_t _4g_send_thread = NULL;


    _4g_send_thread = rt_thread_create("_4G_main_init",
                                       Task4GSend,
                                       RT_NULL,
                                       2048,
                                       16,  		//优先级
                                       20);

    if (_4g_send_thread != RT_NULL)
    {
        rt_thread_startup(_4g_send_thread);
    }
    else
    {
        goto __exit;
    }

    return RT_EOK;

__exit:
    if (_4g_send_thread)
    {
        rt_thread_delete(_4g_send_thread);
    }

    return RT_ERROR;
}

INIT_APP_EXPORT(_4G_send_init);

/************************(C)COPYRIGHT 2022 杭州汇誉*****END OF FILE****************************/

