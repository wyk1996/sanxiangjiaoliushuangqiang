/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			:ModuleYouren.c
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
#include <string.h>
#include <stdlib.h>
#include "ModuleA7680C.h"
#include "4GMain.h"
#include "dwin_com_pro.h"
#define DIM7600_RECV_TABLE_LEN        (22u)
/* Private define-----------------------------------------------------------------------------*/
__packed typedef struct
{
    _4G_STATE SIM_ExistState;                      	//模块是否存在

    _4G_STATE SIM_ConectState[10];					//是否连接到服务器(最大可连接10路)
    _4G_STATE SIM_Regiset[10];						//是否注册成功(最大可连接10路)
    _4G_MODE  Mode;									//4G模式还是HTTP模式

    _4G_STATE HTTP_Start;							//HTTP开始返回
    _4G_STATE HTTP_ConectState;						//HTTP连接状态
    _4G_STATE HTTP_Get;								//HTTP获取
    _4G_STATE HTTP_Read;							//HTTP读取
    _4G_STATE HTTP_ReadDone;						//读取完成
    __packed union
    {
        uint8_t OneByteBits;
        __packed struct
        {
            uint8_t ATCMD_CIMI   	  : 1;			 //读取卡号返回
            uint8_t ATCMD_CSQ  : 1;			 	 //读取信号强度返回

            uint8_t ATCMD_CGDCONT   : 1;			 //配置PDP返回
            uint8_t ATCMD_CIPMODE   : 1;			 //数据透传返回
            uint8_t ATCMD_NETOPEN   : 1;			 //激活PDP返回
            uint8_t ATCMD_SETCIPRXGET  : 1;         //接收手动获取
            uint8_t ATCMD_OK        : 1;           //切换倒指令模式
            uint8_t res        : 1;           //预留
        } OneByte;
    } State0;
    uint8_t ATCMD_CIPOPEN[10];				//连接服务器返回状态
    uint8_t ATCMD_SENDACK[10];				//发送数据确认
} _SIM7600_CMD_STATE;


static uint8_t Send_AT_EnterATCmd(void);
static uint8_t Send_AT_CIPCLOSE(uint8_t num);
static uint8_t Send_AT_NETCLOSE(void);

/* Private typedef----------------------------------------------------------------------------*/
/*static */
_SIM7600_CMD_STATE   SIM7600CmdState;
uint8_t CSQNum = 0;				//信号强度数值

/*****************************************************************************
* Function     : APP_GetCSQNum
* Description  : 获取型号强度值
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t APP_SetSIM7600Mode(_4G_MODE mode)
{
    SIM7600CmdState.Mode = mode;
    return RT_TRUE;
}

/*****************************************************************************
* Function     :Send_AT_HTTPINIT
* Description  :开始HTTP
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_HTTPINIT(void)
{

    if (UART_4GWrite((uint8_t*)"AT+HTTPINIT\r\n", strlen("AT+HTTPINIT\r\n") ) == strlen("AT+HTTPINIT\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}


/*****************************************************************************
* Function     :Send_AT_HTTPTERM
* Description  :结束HTTP
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_HTTPTERM(void)
{

    if (UART_4GWrite((uint8_t*)"AT+HTTPTERM\r\n", strlen("AT+HTTPTERM\r\n") ) == strlen("AT+HTTPTERM\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     :Send_AT_HTTPPAR
* Description  :连接HTTP服务器
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_HTTPPAR(char *http_add)
{
    uint8_t buf[100];
    int length;
    length = snprintf((char *)buf, sizeof(buf),"AT+HTTPPARA=\"URL\",\"%s\"\r\n",http_add);
    if (length == -1)
    {
        rt_kprintf("snprintf error, the len is %d", length);
        return RT_FALSE;
    }
    if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     :Send_AT_GET
* Description  :获取数据
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_GET(void)
{
    if (UART_4GWrite((uint8_t*)"AT+HTTPACTION=0\r\n", strlen("AT+HTTPACTION=0\r\n") ) == strlen("AT+HTTPACTION=1\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}


/*****************************************************************************
* Function     :Send_AT_READ
* Description  :读取数据
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_READ(uint32_t len)
{
    uint8_t buf[60];
    int length;


    length = snprintf((char *)buf, sizeof(buf), "AT+HTTPREAD=0,%d\r\n",len);
    if (length == -1)
    {
        rt_kprintf("snprintf error, the len is %d", length);
        return RT_FALSE;
    }
    if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

///*****************************************************************************
//* Function     :Module_HTTPDownload
//* Description  :HTTP下载
//* Input        :
//* Output       :
//* Return       :
//* Note(s)      :
//* Contributor  : 2018年7月11日
//*****************************************************************************/
//uint8_t Module_HTTPDownload(char * paddr)
//{
//    _RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(0);
//    static uint32_t i = 0;
//    static uint32_t downtime = 0,lastdowntime;
//    uint32_t  down_len = 0;    //下载的长度

//    if(paddr == NULL)
//    {
//        return RT_FALSE;
//    }
//    Send_AT_EnterATCmd();
//    rt_thread_mdelay(CM_TIME_2_SEC);;
//    Send_AT_HTTPTERM();
//    rt_thread_mdelay(CM_TIME_1_SEC);

//    //开始http
//    Send_AT_HTTPINIT();
//    //等待2s回复
//    i = 0;
//    while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
//    {
//        rt_thread_mdelay(CM_TIME_100_MSEC);
//        //等待AT指令被回复
//        if (SIM7600CmdState.HTTP_Start== STATE_OK)
//        {
//            SIM7600CmdState.HTTP_Start = STATE_ERR;
//            break;
//        }
//        i++;
//    }
//    //判断是否回复超时
//    if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
//    {
//        rt_kprintf("respond cmd:HTTPINIT timeout");
//        return RT_FALSE;
//    }

//    //连接http服务器
//    Send_AT_HTTPPAR(paddr);
//    //等待2s回复
//    i = 0;
//    while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
//    {
//        rt_thread_mdelay(CM_TIME_50_MSEC);
//        //等待AT指令被回复
//        if (SIM7600CmdState.HTTP_ConectState== STATE_OK)
//        {
//            SIM7600CmdState.HTTP_ConectState = STATE_ERR;
//            break;
//        }
//        i++;
//    }
//    //判断是否回复超时
//    if (i == CM_TIME_2_SEC / CM_TIME_50_MSEC)
//    {
//        rt_kprintf("respond cmd:HTTPINIT timeout");
//        return RT_FALSE;
//    }

//    //Get
//    Send_AT_GET();
//    //等待2s回复
//    i = 0;
//    while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
//    {
//        rt_thread_mdelay(CM_TIME_50_MSEC);
//        //等待AT指令被回复
//        if (SIM7600CmdState.HTTP_Get== STATE_OK)
//        {
//            SIM7600CmdState.HTTP_Get = STATE_ERR;
//            break;
//        }
//        i++;
//    }
//    //判断是否回复超时
//    if (i == CM_TIME_2_SEC / CM_TIME_50_MSEC)
//    {
//        rt_kprintf("respond cmd:HTTP GET timeout");
//        return RT_FALSE;
//    }


//    downtime =rt_tick_get();
//    lastdowntime = downtime;
//    //read
//    while(!SIM7600CmdState.HTTP_ReadDone)
//    {
//        if(down_len > pcontrol->AllLen)
//        {
//            return RT_FALSE;
//        }
//        if(down_len == pcontrol->AllLen)
//        {
//#warning "校验   YXY 20220531"
//#warning "存储   YXY 20220531"
//            SIM7600CmdState.HTTP_ReadDone = STATE_OK;    //说明下载成功
//            break;
//        }
//        if((pcontrol->AllLen - down_len) > 1000)
//        {
//            Send_AT_READ(1000);   //一次一次读取
//        }
//        else
//        {
//            Send_AT_READ(pcontrol->AllLen - down_len);   //一次一次读取
//        }
//        downtime =rt_tick_get();

//        if(SIM7600CmdState.HTTP_ReadDone== STATE_OK)
//        {
//            //校验，返回下载成功
//            if(down_len == pcontrol->AllLen)
//            {
//#warning "校验   YXY 20220531"
//#warning "存储   YXY 20220531"
//                SIM7600CmdState.HTTP_ReadDone = STATE_OK;    //说明下载成功
//            }
//            break;
//        }
//        if((downtime - lastdowntime)  > (CM_TIME_60_SEC * 2))   //2分钟没又下载完成说明超时了
//        {
//            rt_kprintf("respond cmd:HTTP READ timeout");
//            return RT_FALSE;
//        }


//        //等待2s回复
//        i = 0;
//        while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
//        {
//            rt_thread_mdelay(CM_TIME_50_MSEC);
//            //等待AT指令被回复
//            if (SIM7600CmdState.HTTP_Read== STATE_OK)
//            {
//                SIM7600CmdState.HTTP_Read = STATE_ERR;

//                //数据存储
//                down_len += pcontrol->DownPackLen;
//                break;
//            }
//            i++;
//        }
//        //判断是否回复超时
//        if (i >= (CM_TIME_2_SEC / CM_TIME_50_MSEC) )
//        {
//            rt_kprintf("respond cmd:HTTP READ timeout");
//            return RT_FALSE;
//        }
//    }
//    Send_AT_HTTPTERM();
//    rt_thread_mdelay(CM_TIME_1_SEC);
//    return RT_TRUE;
//}



/*****************************************************************************
* Function     :Module_HTTPDownload
* Description  :HTTP下载
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Module_HTTPDownload(_HTTP_INFO *info)
{
    _RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(0);
    static uint32_t i,a= 0;
    static uint32_t downtime = 0,lastdowntime;
    uint32_t  down_len = 0;    //下载的长度
    uint8_t  down_state = 0x55;		//表示升级成功

    if(info == NULL)
    {
        return FALSE;
    }
    if(info->ServerAdd[0] == 0)
    {
        return FALSE;
    }

	rt_thread_mdelay(CM_TIME_2_SEC);;
    Send_AT_EnterATCmd();
	rt_thread_mdelay(CM_TIME_2_SEC);;
    Send_AT_HTTPTERM();
	rt_thread_mdelay(CM_TIME_1_SEC);;

    //开始http
    Send_AT_HTTPINIT();
    //等待2s回复
    i = 0;
    while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
		rt_thread_mdelay(CM_TIME_100_MSEC);
        //等待AT指令被回复
        if (SIM7600CmdState.HTTP_Start== STATE_OK)
        {
            SIM7600CmdState.HTTP_Start = STATE_ERR;
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
        //printf("respond cmd:HTTPINIT timeout");
        return FALSE;
    }


    //连接http服务器
    Send_AT_HTTPPAR(info->ServerAdd);
    //等待2s回复
    i = 0;
    while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
    {
        //OSTimeDly(CM_TIME_50_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);
		rt_thread_mdelay(CM_TIME_50_MSEC);
        //等待AT指令被回复
        if (SIM7600CmdState.HTTP_ConectState== STATE_OK)
        {
            SIM7600CmdState.HTTP_ConectState = STATE_ERR;
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i == CM_TIME_2_SEC / CM_TIME_50_MSEC)
    {
        //printf("respond cmd:Send_AT_HTTPPAR timeout");
        return FALSE;
    }



    //Get
    Send_AT_GET();
    //等待2s回复
    i = 0;
    while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
    {
        //OSTimeDly(CM_TIME_50_MSEC, OS_OPT_TIME_PERIODIC, &timeerr);
		rt_thread_mdelay(CM_TIME_50_MSEC);
        //等待AT指令被回复
        if (SIM7600CmdState.HTTP_Get== STATE_OK)
        {
            SIM7600CmdState.HTTP_Get = STATE_ERR;
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i == CM_TIME_2_SEC / CM_TIME_50_MSEC)
    {
        rt_kprintf("respond cmd:Send_AT_GET timeout");
        return FALSE;
    }

    if( pcontrol->AllLen > (1024*200) )   //数据总长度不能操作200k
    {
        rt_kprintf("http data too len");
        return FALSE;
    }



    downtime =rt_tick_get();
    lastdowntime = downtime;
    //read
    while(!SIM7600CmdState.HTTP_ReadDone)
    {
        if(down_len > pcontrol->AllLen)
        {
            return FALSE;
        }
        if(down_len == pcontrol->AllLen)
        {
			fal_partition_APPOTA(1,(uint8_t*)&down_len,sizeof(down_len)); 
            SIM7600CmdState.HTTP_ReadDone = STATE_OK;    //说明下载成功
			fal_partition_APPOTA(0,&down_state,sizeof(down_state));
            break;
        }
        if((pcontrol->AllLen - down_len) > 500)
        {
            Send_AT_READ(500);   //一次一次读取
        }
        else
        {
            Send_AT_READ(pcontrol->AllLen - down_len);   //一次一次读取
        }
        downtime =rt_tick_get();

        if(SIM7600CmdState.HTTP_ReadDone== STATE_OK)
        {
            //校验，返回下载成功
            if(down_len == pcontrol->AllLen)
            {
				fal_partition_APPOTA(1,(uint8_t*)&down_len,sizeof(down_len));
                SIM7600CmdState.HTTP_ReadDone = STATE_OK;    //说明下载成功
				fal_partition_APPOTA(0,&down_state,sizeof(down_state));
            }
            break;
        }
        if((downtime - lastdowntime)  > (CM_TIME_60_SEC * 2))   //2分钟没又下载完成说明超时了
        {
            rt_kprintf("respond cmd:HTTP READ timeout");
            return FALSE;
        }


        //等待2s回复
        i = 0;
        while (i < CM_TIME_2_SEC / CM_TIME_50_MSEC)
        {
			rt_thread_mdelay(CM_TIME_50_MSEC);
            //等待AT指令被回复
            if (SIM7600CmdState.HTTP_Read== STATE_OK)
            {
                SIM7600CmdState.HTTP_Read = STATE_ERR;

                //数据存储
				fal_partition_APPOTA(down_len + 5,pcontrol->DataBuf,pcontrol->DownPackLen);
                down_len += pcontrol->DownPackLen;
                break;
            }
            i++;
        }
        //判断是否回复超时
        if (i >= (CM_TIME_2_SEC / CM_TIME_50_MSEC) )
        {
            //rt_kprintf("respond cmd:HTTP READ timeout");
            return FALSE;
        }
    }
    Send_AT_HTTPTERM();  //关闭http服务
	rt_thread_mdelay(CM_TIME_1_SEC);
    return TRUE;

}























/*****************************************************************************
* Function     : APP_GetCSQNum
* Description  : 获取型号强度值
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t APP_GetCSQNum(void)
{
    return CSQNum;
}



//以下是AT指令发送处理
/*****************************************************************************
* Function     : SIM7600Reset
* Description  : 模块复位
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_MODE APP_GetSIM7600Mode(void)
{
    return SIM7600CmdState.Mode;
}

//以下是AT指令发送处理
/*****************************************************************************
* Function     : SIM7600Reset
* Description  : 模块复位
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t SIM7600Reset()
{
    static uint8_t Power_count=0;   //电源断电次数
    uint8_t i;
    memset(&SIM7600CmdState,0,sizeof(SIM7600CmdState));
    for(i = 0; i < NetConfigInfo[NET_YX_SELCT].NetNum; i++)
    {
        Send_AT_CIPCLOSE(i);
    }
    Send_AT_NETCLOSE();

    rt_kprintf("4G reset \r\n");
    if(++Power_count > 3)
    {
        //电源引脚复位 5s电容泄放
        rt_pin_write(IO_4G_PWR,PIN_LOW);
        rt_thread_mdelay(5000);
        rt_pin_write(IO_4G_PWR,PIN_HIGH);
        rt_thread_mdelay(250);
    }

    rt_pin_write(IO_4G_RES,PIN_LOW);    //复位引脚
    rt_pin_write(IO_4G_ONOFF,PIN_LOW);   //开机
    rt_thread_mdelay(250);
		

    //复位
    rt_pin_write(IO_4G_RES,PIN_HIGH);
    rt_thread_mdelay(3000);
    rt_pin_write(IO_4G_RES,PIN_LOW);
    rt_thread_mdelay(500);

    rt_thread_mdelay(CM_TIME_10_SEC);
    return RT_FALSE;
}

/*****************************************************************************
* Function     : SIM7600CloseNet
* Description  : 关闭网络
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t SIM7600CloseNet(uint8_t num)
{
    if(num > 10)
    {
        return RT_FALSE;
    }

    Send_AT_CIPCLOSE(num);
    return RT_TRUE;
}

/*****************************************************************************
* Function     : APP_GetSIM7600Status
* Description  : 获取模块是否存在
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetSIM7600Status(void)
{
    return SIM7600CmdState.SIM_ExistState;
}



/*****************************************************************************
* Function     : APP_GetModuleConnectState
* Description  : 连接服务器状态,最大可连接10路
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetModuleConnectState(uint8_t num)
{
    if(num > 10)
    {
        return STATE_ERR;
    }
    return SIM7600CmdState.SIM_ConectState[num];
}

/*****************************************************************************
* Function     : APP_GetAppRegisterState
* Description  : 注册是否成功,最大可连接10路
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetAppRegisterState(uint8_t num)
{
    if(num > 10)
    {
        return STATE_ERR;
    }
    return SIM7600CmdState.SIM_Regiset[num];
}

/*****************************************************************************
* Function     : APP_GetAppRegisterState
* Description  : 注册是否成功,最大可连接10路
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t  APP_SetAppRegisterState(uint8_t num,_4G_STATE state)
{
    if(num > 10)
    {
        return RT_FALSE;
    }
    SIM7600CmdState.SIM_Regiset[num] = state;
}


/*****************************************************************************
* Function     : Send_AT_CIPCLOSE
* Description  : 关闭服务器
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_CIPCLOSE(uint8_t num)
{
    uint8_t buf[60];
    int length;
    if(num> 10)
    {
        return RT_FALSE;
    }

    length = snprintf((char *)buf, sizeof(buf), "AT+CIPCLOSE=%d\r\n",num);
    if (length == -1)
    {
        rt_kprintf("snprintf error, the len is %d", length);
        return RT_FALSE;
    }
    if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
        return RT_TRUE;
    }

    return RT_FALSE;
}


/*****************************************************************************
* Function     : Send_AT_NETCLOSE
* Description  : 去除pdp
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_NETCLOSE(void)
{
    if (UART_4GWrite((uint8_t*)"AT+NETCLOSE\r\n", strlen("AT+NETCLOSE\r\n") ) == strlen("AT+NETCLOSE\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     : Send_AT_EnterATCmd
* Description  : 进入AT配置指令1
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_EnterATCmd(void)
{
    if (UART_4GWrite((uint8_t*)"+++", strlen("+++") ) == strlen("+++") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}
/*****************************************************************************
* Function     : Send_AT_CSQ
* Description  : 读取信号强度
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Send_AT_CSQ(void)
{
    if (UART_4GWrite((uint8_t*)"AT+CSQ\r\n", strlen("AT+CSQ\r\n") ) == strlen("AT+CSQ\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     : Send_AT_CIMI
* Description  : 读取卡号
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_CIMI(void)
{
    if (UART_4GWrite((uint8_t*)"AT+CIMI\r\n", strlen("AT+CIMI\r\n") ) == strlen("AT+CIMI\r\n") )
        //if(BSP_UARTWrite(GPRS_UART,(uint8_t*)"AT+CIMI\r\n",strlen("AT+CIMI\r\n")))
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}
/*****************************************************************************
* Function     : Send_AT_CGDCONT
* Description  : 配置PDP
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_CGDCONT(void)
{
    if (UART_4GWrite((uint8_t*)"AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n", strlen("AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n") ) == strlen("AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}
/*****************************************************************************
* Function     : Send_AT_CIPMODE
* Description  : 数据透传
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Send_AT_CIPMODE(void)
{
    if (UART_4GWrite((uint8_t*)"AT+CIPMODE=1\r\n", strlen("AT+CIPMODE=1\r\n") ) == strlen("AT+CIPMODE=1\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     : Send_AT_NOTCIPMODE
* Description  : 非数据透传
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_NOTCIPMODE(void)
{
    if (UART_4GWrite((uint8_t*)"AT+CIPMODE=0\r\n", strlen("AT+CIPMODE=0\r\n") ) == strlen("AT+CIPMODE=0\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     : Send_AT_CIPRXGET
* Description  : 接收手动获取
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_SetCIPRXGET(void)
{
    if (UART_4GWrite((uint8_t*)"AT+CIPRXGET=1\r\n", strlen("AT+CIPRXGET=1\r\n") ) == strlen("AT+CIPMODE=0\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     : Send_AT_NETOPEN
* Description  : 激活PDP
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_NETOPEN(void)
{
    if (UART_4GWrite((uint8_t*)"AT+NETOPEN\r\n", strlen("AT+NETOPEN\r\n") ) == strlen("AT+NETOPEN\r\n") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     : Send_AT_CIPSEND
* Description  : 发送数据请求
* Input        :num  哪个socket
				len   数据长度
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Send_AT_CIPSEND(uint8_t num,uint8_t len)
{
    uint8_t buf[60];
    int length;

    length = snprintf((char *)buf, sizeof(buf), "AT+CIPSEND=%d,%d\r\n",num,len);
    if (length == -1)
    {
        rt_kprintf("snprintf error, the len is %d", length);
        return RT_FALSE;
    }
    if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     : Send_AT_CIPRXGET
* Description  : 读取数据请求
* Input        :num  哪个socket
				len   数据长度
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Send_AT_CIPRXGET(uint8_t num)
{
    uint8_t buf[60];
    int length;

    length = snprintf((char *)buf, sizeof(buf), "AT+CIPRXGET=2,%d\r\n",num);
    if (length == -1)
    {
        rt_kprintf("snprintf error, the len is %d", length);
        return RT_FALSE;
    }
    if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}

/*****************************************************************************
* Function     :Module_ConnectServer
* Description  :连接服务器
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Module_ConnectServer(uint8_t num,uint8_t* pIP,uint16_t port)
{
    uint8_t buf[60];
    int length;
    if((pIP == NULL) || (num> 10))
    {
        return RT_FALSE;
    }

    length = snprintf((char *)buf, sizeof(buf), "AT+CIPOPEN=%d,\"TCP\",\"%s\",%d\r\n",num,pIP,port);
    if (length == -1)
    {
        rt_kprintf("snprintf error, the len is %d", length);
        return RT_FALSE;
    }
    if (UART_4GWrite(buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}



//以下是AT指令接收数据处理
/*****************************************************************************
* Function     : Recv_AT_EnterATCmd_Ack
* Description  : 进入AT配置指令返回
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_EnterATCmd_Ack(uint8_t *pdata, uint16_t len)
{
    if (UART_4GWrite((uint8_t*)"+++", strlen("+++") ) == strlen("+++") )
    {
        return RT_TRUE;
    }
    return RT_FALSE;
}


/*****************************************************************************
* Function     : Recv_AT_CSQ_Ack
* Description  : 读取信号强度返回
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_CSQ_Ack(uint8_t *pdata, uint16_t len)
{

    if ( (pdata == NULL) || len < 16)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return RT_FALSE;
    }

    CSQNum = atoi((const char *)&pdata[14]);
    if((CSQNum == 99) || (CSQNum == 0) )
    {
        //无信号
        return RT_FALSE;
    }
    rt_enter_critical();
    SIM7600CmdState.State0.OneByte.ATCMD_CSQ = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}

/*****************************************************************************
* Function     : Recv_AT_CIMI_Ack
* Description  : 读取卡号返回
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_CIMI_Ack(uint8_t *pdata, uint16_t len)
{
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return RT_FALSE;
    }
    rt_enter_critical();
    SIM7600CmdState.State0.OneByte.ATCMD_CIMI = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}
/*****************************************************************************
* Function     : Recv_AT_CGDCONT_Ack
* Description  : 配置PDP返回
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_CGDCONT_Ack(uint8_t *pdata, uint16_t len)
{
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
//	if (strstr((char *)pdata, "OK") == NULL)
//    {
//        return FALSE;
//    }
    rt_enter_critical();
    SIM7600CmdState.State0.OneByte.ATCMD_CGDCONT = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}
/*****************************************************************************
* Function     : Recv_AT_CIPMODE_Ack
* Description  : 数据透传返回
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_CIPMODE_Ack(uint8_t *pdata, uint16_t len)
{
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
//	if (strstr((char *)pdata, "OK") == NULL)
//    {
//        return FALSE;
//    }
    rt_enter_critical();
    SIM7600CmdState.State0.OneByte.ATCMD_CIPMODE = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}

/*****************************************************************************
* Function     : Recv_AT_NETOPEN_Ack
* Description  : 激活PDP返回
* Input        : uint8_t *pdata
                 uint16_t len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_NETOPEN_Ack(uint8_t *pdata, uint16_t len)
{

    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
//	if (strstr((char *)pdata, "OK") == NULL)
//    {
//        return FALSE;
//    }
    rt_enter_critical();
    SIM7600CmdState.State0.OneByte.ATCMD_NETOPEN = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}


/*****************************************************************************
* Function     :Recv_AT_ConnectServer_Ack
* Description  :连接服务器返回
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ConnectServer0_Ack(uint8_t *pdata, uint16_t len)
{
//	AT+CIPOPEN=0,"TCP","122.114.122.174",33870
//	OK

//	+CIPOPEN: 0,0

    uint8_t res;
    char *poffset;

    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
    if (strstr((char *)pdata, "OK") != NULL)
    {
        rt_enter_critical();
        SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
        rt_exit_critical();
        return RT_TRUE;
    }

    poffset = strtok((char *)pdata, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }

    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    res = atoi(poffset);
    //0表示新连接上，4表示意见连接上了
    if((res != 0) || (res != 4))
    {
        return RT_FALSE;
    }
    rt_enter_critical();
    SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}

/*****************************************************************************
* Function     :Recv_AT_ConnectServer_Ack
* Description  :连接服务器返回
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ConnectServer1_Ack(uint8_t *pdata, uint16_t len)
{
    //	AT+CIPOPEN=0,"TCP","122.114.122.174",33870
//	OK

//	+CIPOPEN: 0,0

    uint8_t res;
    char *poffset;
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
    if (strstr((char *)pdata, "OK") != NULL)
    {
        rt_enter_critical();
        SIM7600CmdState.ATCMD_CIPOPEN[1] = _4G_RESPOND_OK;
        rt_exit_critical();
        return RT_TRUE;
    }

    poffset = strtok((char *)pdata, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }

    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    res = atoi(poffset);
    //0表示新连接上，4表示意见连接上了
    if((res != 0) || (res != 4))
    {
        return RT_FALSE;
    }
    rt_enter_critical();
    SIM7600CmdState.ATCMD_CIPOPEN[1] = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}


/*****************************************************************************
* Function     :Recv_AT_CmdConnectServer0_Ack
* Description  :连接服务器指令返回
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ConnectServer0Cmd_Ack(uint8_t *pdata, uint16_t len)
{
    //	AT+CIPOPEN=0,"TCP","122.114.122.174",33870
//	OK

//	+CIPOPEN: 0,0

    uint8_t res;
    char *poffset;
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }

    pdata[len] = '\0';  //结束符
    if(NetConfigInfo[NET_YX_SELCT].NetNum == 1)
    {
        {
            if (strstr((char *)pdata, "ERROR") == NULL)  //没有错误就返回注册成功
            {
                rt_enter_critical();
                SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
                rt_exit_critical();
                return RT_TRUE;
            }
            return RT_FALSE;
        }
    }
    else
    {
        if (strstr((char *)pdata, "ERROR") != NULL)
        {
            return RT_FALSE;
        }

        if (strstr((char *)pdata, "OK") != NULL)
        {
            rt_enter_critical();
            SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
            rt_exit_critical();
            return RT_TRUE;
        }

        poffset = strtok((char *)pdata, ",");
        if(poffset == NULL)
        {
            return RT_FALSE;
        }

        poffset = strtok(NULL, ",");
        if(poffset == NULL)
        {
            return RT_FALSE;
        }

        poffset = strtok(NULL, ",");
        if(poffset == NULL)
        {
            return RT_FALSE;
        }

        poffset = strtok(NULL, ",");
        if(poffset == NULL)
        {
            return RT_FALSE;
        }
        res = atoi(poffset);
        //0表示新连接上，4表示意见连接上了
        if((res != 0) || (res != 4))
        {
            return RT_FALSE;
        }
        rt_enter_critical();
        SIM7600CmdState.ATCMD_CIPOPEN[0] = _4G_RESPOND_OK;
        rt_exit_critical();
    }
    return RT_TRUE;
}

/*****************************************************************************
* Function     :Recv_AT_CmdConnectServer0_Ack
* Description  :连接服务器指令返回
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ConnectServer1Cmd_Ack(uint8_t *pdata, uint16_t len)
{
    //	AT+CIPOPEN=0,"TCP","122.114.122.174",33870
//	OK

//	+CIPOPEN: 0,0


    uint8_t res;
    char *poffset;
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
    if (strstr((char *)pdata, "OK") != NULL)
    {
        rt_enter_critical();
        SIM7600CmdState.ATCMD_CIPOPEN[1] = _4G_RESPOND_OK;
        rt_exit_critical();
        return RT_TRUE;
    }

    if (strstr((char *)pdata, "ERROR") != NULL)
    {
        return RT_FALSE;
    }

    poffset = strtok((char *)pdata, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }

    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    res = atoi(poffset);
    //0表示新连接上，4表示意见连接上了
    if((res != 0) || (res != 4))
    {
        return RT_FALSE;
    }
    rt_enter_critical();
    SIM7600CmdState.ATCMD_CIPOPEN[1] = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}
/*****************************************************************************
* Function     :Recv_AT_SetReAct_Ack
* Description  :设置接收主动获取
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_SetReAct_Ack(uint8_t *pdata, uint16_t len)
{
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
//	if (strstr((char *)pdata, "OK") == NULL)
//    {
//        return FALSE;
//    }
    rt_enter_critical();
    SIM7600CmdState.State0.OneByte.ATCMD_SETCIPRXGET = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}


/*****************************************************************************
* Function     :Recv_AT_SendAck0Cmd_Ack
* Description  :发送数据确认
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_SendAck0Cmd_Ack(uint8_t *pdata, uint16_t len)
{
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    //
    pdata[len] = '\0';  //结束符
//	if ((strstr((char *)pdata, ">") == NULL) )
//    {
//		rt_enter_critical();
//		SIM7600CmdState.ATCMD_SENDACK[0] = _4G_RESPOND_AGAIN;
//		OSSchedUnlock();
//        return FALSE;
//    }
    if ((strstr((char *)pdata, "ERROR") != NULL) )
    {
        rt_enter_critical();
        SIM7600CmdState.ATCMD_SENDACK[0] = _4G_RESPOND_AGAIN;
        rt_exit_critical();
        return RT_FALSE;
    }
    rt_enter_critical();
    SIM7600CmdState.ATCMD_SENDACK[0] = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}

/*****************************************************************************
* Function     :Recv_AT_SendAck0Cmd_Ack
* Description  :发送数据确认
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_SendAck1Cmd_Ack(uint8_t *pdata, uint16_t len)
{
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    //
    pdata[len] = '\0';  //结束符
    //	if ((strstr((char *)pdata, ">") == NULL) )
//    {
//		rt_enter_critical();
//		SIM7600CmdState.ATCMD_SENDACK[1] = _4G_RESPOND_AGAIN;
//		rt_exit_critical();
//        return FALSE;
//    }
    if ((strstr((char *)pdata, "ERROR") != NULL) )
    {
        rt_enter_critical();
        SIM7600CmdState.ATCMD_SENDACK[1] = _4G_RESPOND_AGAIN;
        rt_exit_critical();
        return RT_FALSE;
    }
    rt_enter_critical();
    SIM7600CmdState.ATCMD_SENDACK[1] = _4G_RESPOND_OK;
    rt_exit_critical();
    return RT_TRUE;
}



/*****************************************************************************
* Function     :Recv_AT_SendAck0Cmd_Ack
* Description  :发送数据确认
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ReRecv0Cmd_Ack(uint8_t *pdata, uint16_t len)
{

    _RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(0);
    if(pcontrol == NULL)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
    if ((strstr((char *)pdata, "+IP ERROR: No data") != NULL) )
    {
        //表示无数据
        return RT_FALSE;
    }
//	AT+CIPRXGET=2,0
//	+CIPRXGET: 2,0,6,0
//	123456

//	OK
//	if ((strstr((char *)pdata, "OK") == NULL) )
//   {
//		//表示无数据
//       return FALSE;
//   }
    char *poffset = strtok((char *)pdata + 18, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");  //为数据长度
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    pcontrol->len = atoi(poffset);
    if(pcontrol->len > 500)
    {
        return RT_FALSE;
    }
    //数据拷贝
    if(pcontrol->len < 10)
    {
        memcpy(pcontrol->DataBuf,poffset+5,pcontrol->len);
    }
    else if(pcontrol->len < 100)
    {
        memcpy(pcontrol->DataBuf,poffset+6,pcontrol->len);
    }
    else
    {
        memcpy(pcontrol->DataBuf,poffset+7,pcontrol->len);
    }
    pcontrol->RecvStatus = RECV_FOUND_DATA;  //有数据了，因为这个变量再同一个任务，故不要加锁
    return RT_TRUE;
}

/*****************************************************************************
* Function     :Recv_AT_SendAck0Cmd_Ack
* Description  :发送数据确认
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_AT_ReRecv1Cmd_Ack(uint8_t *pdata, uint16_t len)
{
    _RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(1);
    if(pcontrol == NULL)
    {
        return RT_FALSE;
    }
    pdata[len] = '\0';  //结束符
    if ((strstr((char *)pdata, "+IP ERROR: No data") != NULL) )
    {
        //表示无数据
        return RT_FALSE;
    }
//	AT+CIPRXGET=2,0
//	+CIPRXGET: 2,0,6,0
//	123456

//	OK
//	if ((strstr((char *)pdata, "OK") == NULL) )
//   {
//		//表示无数据
//       return FALSE;
//   }
    char *poffset = strtok((char *)pdata + 18, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");  //为数据长度
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    pcontrol->len = atoi(poffset);
    if(pcontrol->len > 500)
    {
        return RT_FALSE;
    }
    //数据拷贝
    if(pcontrol->len < 10)
    {
        memcpy(pcontrol->DataBuf,poffset+5,pcontrol->len);
    }
    else if(pcontrol->len < 100)
    {
        memcpy(pcontrol->DataBuf,poffset+6,pcontrol->len);
    }
    else
    {
        memcpy(pcontrol->DataBuf,poffset+7,pcontrol->len);
    }
    pcontrol->RecvStatus = RECV_FOUND_DATA;  //有数据了，因为这个变量再同一个任务，故不要加锁
    return RT_TRUE;
}



/*****************************************************************************
* Function     :Recv_ActRecv0_Ack
* Description  ：有数据接收
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_ActRecv0_Ack(uint8_t *pdata, uint16_t len)
{
    pdata = pdata;
    len = len;

    Send_AT_CIPRXGET(0);		//去读取数据
    return RT_TRUE;
}

/*****************************************************************************
* Function     :Recv_ActRecv1_Ack
* Description  ：有数据接收
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_ActRecv1_Ack(uint8_t *pdata, uint16_t len)
{
    pdata = pdata;
    len = len;

    Send_AT_CIPRXGET(1);		//去读取数据
    return RT_TRUE;
}
/*****************************************************************************
* Function     :Recv_ActRecv1_Ack
* Description  ：有数据接收
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_HttpStart_Ack(uint8_t *pdata, uint16_t len)
{

//[09:13:46.037]发→◇AT+HTTPINIT
//□
//[09:13:46.048]收←◆AT+HTTPINIT
//OK

    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    //
    pdata[len] = '\0';  //结束符
    if ((strstr((char *)pdata, "OK") == NULL) )
    {
        //表示不成功
        return RT_FALSE;
    }

    rt_enter_critical();
    SIM7600CmdState.HTTP_Start = STATE_OK;
    rt_exit_critical();

    return RT_TRUE;
}

/*****************************************************************************
* Function     :Recv_HttpConect_Ack
* Description  ：连接返回
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_HttpConect_Ack(uint8_t *pdata, uint16_t len)
{

//[09:14:48.781]发→◇AT+HTTPPARA="URL","http://14.205.92.144/dowload/project.hex"
//□
//[09:14:48.798]收←◆AT+HTTPPARA="URL","http://14.205.92.144/dowload/project.hex"
//OK
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    //
    pdata[len] = '\0';  //结束符
    if ((strstr((char *)pdata, "OK") == NULL) )
    {
        //表示不成功
        return RT_FALSE;
    }
    rt_enter_critical();
    SIM7600CmdState.HTTP_ConectState = STATE_OK;
    rt_exit_critical();
    return RT_TRUE;
}

/*****************************************************************************
* Function     :Recv_HttpConect_Ack
* Description  ：连接返回
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_HttpGet_Ack(uint8_t *pdata, uint16_t len)
{
//[09:15:18.284]发→◇AT+HTTPACTION=0
//□
//[09:15:18.291]收←◆AT+HTTPACTION=0
//OK

//[09:15:18.654]收←◆
//+HTTPACTION: 0,200,440836
    _RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(0);
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    //
    pdata[len] = '\0';  //结束符

//	if ((strstr((char *)pdata, "OK") == NULL) )
//	{
//		//表示不成功
//		return RT_FALSE;
//	}

    char *poffset = strtok((char *)pdata, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    poffset = strtok(NULL, ",");
    if(poffset == NULL)
    {
        return RT_FALSE;
    }
    pcontrol->AllLen = atoi((char*)poffset);
    if(pcontrol->AllLen == 0)
    {
        return RT_FALSE;
    }
    rt_enter_critical();
    SIM7600CmdState.HTTP_Get = STATE_OK;
    rt_exit_critical();
    return RT_TRUE;
}



/*****************************************************************************
* Function     :Recv_ReadData_Ack
* Description  ：读取数据
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_ReadData_Ack(uint8_t *pdata, uint16_t len)
{
//AT+HTTPREAD=0,1000
//OK

//+HTTPREAD: 1000
    char * pchar  = "+HTTPREAD:";
    uint16_t datalen;
    _RECV_DATA_CONTROL* pcontrol = APP_RecvDataControl(0);
    if ( (pdata == NULL) || !len)
    {
        return RT_FALSE;
    }
    //
    pdata[len] = '\0';  //结束符

    if ((strstr((char *)pdata, "OK") == NULL) )
    {
        //表示不成功
        return RT_FALSE;
    }
    char *poffset;


    //38
    poffset = pdata + 14;


    datalen = atoi((char*)poffset);
    if((datalen == 0) || (datalen > 1024) )
    {
        return RT_FALSE;
    }
    if (datalen >= 1000)
    {
        poffset += 14;
    }
    else if (datalen >= 100)
    {
        poffset += 13;
    }
    else if (datalen >= 10)
    {
        poffset += 12;
    }
    else
    {
        poffset += 11;
    }







    datalen = 0;


    poffset += strlen(pchar);

    datalen= atoi((char*)poffset);
    if((datalen == 0) || (datalen > 1024) )
    {
        return RT_FALSE;
    }
    pcontrol->len = datalen;
    if (pcontrol->len >= 1000)
    {
        poffset += 6;
    }
    else if (pcontrol->len >= 100)
    {
        poffset += 5;
    }
    else if (pcontrol->len >= 10)
    {
        poffset += 4;
    }
    else
    {
        poffset += 3;
    }


    memcpy(pcontrol->DataBuf,poffset,pcontrol->len);   //拷贝数据
    pcontrol->DownPackLen = pcontrol->len;           //单包的长度
    rt_enter_critical();
    SIM7600CmdState.HTTP_Read = STATE_OK;
    rt_exit_critical();
    return RT_TRUE;
}

/*****************************************************************************
* Function     :Recv_HttpConect_Ack
* Description  ：连接返回
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
static uint8_t Recv_ReadDone_Ack(uint8_t *pdata, uint16_t len)
{
//[15:51:43.864]收←◆
//+HTTPREAD: 0
    rt_enter_critical();
    SIM7600CmdState.HTTP_ReadDone = STATE_OK;
    rt_exit_critical();
    return RT_TRUE;
}


//GPRS接收表
static const _4G_AT_FRAME 	TIM7600_RecvFrameTable[DIM7600_RECV_TABLE_LEN] =
{

    {"AT+CSQ"						,      Recv_AT_CSQ_Ack							},		//读取信号强度返回

    {"AT+CIMI"         			    ,  	   Recv_AT_CIMI_Ack							},		//读取卡号返回

    {"AT+CGDCONT"                   ,      Recv_AT_CGDCONT_Ack                    	},		//配置PDP返回

    {"AT+CIPMODE"                 	,      Recv_AT_CIPMODE_Ack                       	},		//数据透传返回

    {"AT+NETOPEN"                 	,      Recv_AT_NETOPEN_Ack                       	},		//激活PDP

    {"AT+CIPOPEN=0"                 ,      Recv_AT_ConnectServer0Cmd_Ack               },		//连接服务器指令返回

    {"AT+CIPOPEN=1"                 ,      Recv_AT_ConnectServer1Cmd_Ack               },		//连接服务器指令返回


    {"+CIPOPEN: 0"                 ,      Recv_AT_ConnectServer0_Ack               },		//连接服务器返回

    {"+CIPOPEN: 1"                 ,      Recv_AT_ConnectServer1_Ack               },		//连接服务器返回

    {"OK"							,      Recv_AT_EnterATCmd_Ack					},		//进入AT配置指令返回

    {"AT+CIPSEND=0"					,      Recv_AT_SendAck0Cmd_Ack					},		//发送数据确认

    {"AT+CIPSEND=1"					,      Recv_AT_SendAck1Cmd_Ack					},		//发送数据确认

    {"AT+CIPRXGET=2,0"				,      Recv_AT_ReRecv0Cmd_Ack					},		//接收返回

    {"AT+CIPRXGET=2,1"				,      Recv_AT_ReRecv1Cmd_Ack					},		//接收返回

    {"AT+CIPRXGET=1"				,      Recv_AT_SetReAct_Ack						},		//设置接收主动获取

    {"\r\n+CIPRXGET: 1,0"			,	   Recv_ActRecv0_Ack						},		//0号主动接收数据

    {"\r\n+CIPRXGET: 1,1"			,	   Recv_ActRecv1_Ack						},		//1号主动接收数据
    //HTTP
    {"AT+HTTPINIT"					,		Recv_HttpStart_Ack						},		//开始返回

    {"AT+HTTPPARA=\"URL\""			,		Recv_HttpConect_Ack						},		//注册返回

    {"\r\n+HTTPACTION:"				,		Recv_HttpGet_Ack						},		//读取数据返回长度

    {"\r\n+HTTPREAD: 0"				,		Recv_ReadDone_Ack							},		//读取完成

    {"AT+HTTPREAD=0"				,		Recv_ReadData_Ack							},		//读取数据

};



/*****************************************************************************
* Function     :Module_SIM7600Test
* Description  :模块是否存在
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Module_SIM7600Test(void)
{
    static uint8_t i = 0;
    uint8_t err;
    static uint8_t count = 1;

    //读取卡号
    Send_AT_CIMI();
    //等待2s回复
    i = 0;
    while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
        rt_thread_mdelay(CM_TIME_100_MSEC);
        //等待AT指令被回复
        if (SIM7600CmdState.State0.OneByte.ATCMD_CIMI == _4G_RESPOND_OK)
        {
            SIM7600CmdState.State0.OneByte.ATCMD_CIMI = _4G_RESPOND_ERROR;
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
        rt_kprintf("respond cmd:ATCMD_CIMI timeout");
        return RT_FALSE;
    }


    //读取信号强度
    Send_AT_CSQ();
    //等待2s回复
    i = 0;
    while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
        rt_thread_mdelay(CM_TIME_100_MSEC);
        //等待AT指令被回复
        if (SIM7600CmdState.State0.OneByte.ATCMD_CSQ == _4G_RESPOND_OK)
        {
            SIM7600CmdState.State0.OneByte.ATCMD_CSQ = _4G_RESPOND_ERROR;
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
        rt_kprintf("respond cmd:ATCMD_CSQ timeout");
        return RT_FALSE;
    }

    //配置pdp
    Send_AT_CGDCONT();
    //等待2s回复
    i = 0;
    while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
        rt_thread_mdelay(CM_TIME_100_MSEC);
        //等待AT指令被回复
        if (SIM7600CmdState.State0.OneByte.ATCMD_CGDCONT == _4G_RESPOND_OK)
        {
            SIM7600CmdState.State0.OneByte.ATCMD_CGDCONT = _4G_RESPOND_ERROR;
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
        rt_kprintf("respond cmd:ATCMD_CGDCONT timeout");
        return RT_FALSE;
    }

    //设置为透传模式
    if(NetConfigInfo[NET_YX_SELCT].NetNum == 1)
    {

        Send_AT_CIPMODE();
    }
    else
    {
        //设置数据为非透传
        Send_AT_NOTCIPMODE();
    }
    rt_thread_mdelay(CM_TIME_500_MSEC);
    i = 0;
    while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
        rt_thread_mdelay(CM_TIME_100_MSEC);
        //等待AT指令被回复
        if (SIM7600CmdState.State0.OneByte.ATCMD_CIPMODE == _4G_RESPOND_OK)
        {
            SIM7600CmdState.State0.OneByte.ATCMD_CIPMODE = _4G_RESPOND_ERROR;
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
    {
        rt_kprintf("respond cmd:ATCMD_CIPMODE timeout");
        return RT_FALSE;
    }

    if(NetConfigInfo[NET_YX_SELCT].NetNum > 1)
    {
        Send_AT_SetCIPRXGET(); //接收手动获取
        i = 0;
        while (i < CM_TIME_2_SEC / CM_TIME_100_MSEC)
        {
            rt_thread_mdelay(CM_TIME_100_MSEC);
            //等待AT指令被回复
            if (SIM7600CmdState.State0.OneByte.ATCMD_SETCIPRXGET == _4G_RESPOND_OK)
            {
                SIM7600CmdState.State0.OneByte.ATCMD_SETCIPRXGET = _4G_RESPOND_ERROR;
                break;
            }
            i++;
        }
        //判断是否回复超时
        if (i == CM_TIME_2_SEC / CM_TIME_100_MSEC)
        {
            rt_kprintf("respond cmd:ATCMD_CIPMODE timeout");
            return RT_FALSE;
        }
    }

    SIM7600CmdState.SIM_ExistState = STATE_OK;		//模块存在

    return RT_TRUE;
}

/*****************************************************************************
* Function     :ModuleSIM7600_ConnectServer
* Description  :连接服务器
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t ModuleSIM7600_ConnectServer(uint8_t num,uint8_t* pIP,uint16_t port)
{
    static uint8_t i = 0;
    //激活PDP
    Send_AT_NETOPEN();
    i = 0;
    while (i < (CM_TIME_2_SEC / CM_TIME_100_MSEC) )
    {
        rt_thread_mdelay(CM_TIME_100_MSEC);
        //等待AT指令被回复
        if (SIM7600CmdState.State0.OneByte.ATCMD_NETOPEN == _4G_RESPOND_OK)
        {
            SIM7600CmdState.State0.OneByte.ATCMD_NETOPEN = _4G_RESPOND_ERROR;
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i >= (CM_TIME_2_SEC / CM_TIME_100_MSEC) )
    {
        rt_kprintf("respond cmd:ATCMD_NETOPEN timeout");
        return RT_FALSE;
    }


    Module_ConnectServer(num,pIP,port);
    i = 0;
    while (i < (CM_TIME_5_SEC / CM_TIME_100_MSEC) )
    {
        rt_thread_mdelay(CM_TIME_100_MSEC);
        //等待AT指令被回复
        if (SIM7600CmdState.ATCMD_CIPOPEN[num] == _4G_RESPOND_OK)
        {
            SIM7600CmdState.ATCMD_CIPOPEN[num] = _4G_RESPOND_ERROR;
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i >= (CM_TIME_5_SEC / CM_TIME_100_MSEC))
    {
        rt_kprintf("respond cmd:ATCMD_CIPOPEN timeout");
        return RT_FALSE;
    }
    //rt_thread_mdelay(CM_TIME_10_SEC);
    SIM7600CmdState.SIM_ConectState[num] = STATE_OK;		//已经连接上了服务器
    return RT_TRUE;
}


/*****************************************************************************
* Function     :ModuleSIM7600_SendData
* Description  :发送数据
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t ModuleSIM7600_SendData(uint8_t num,uint8_t* pdata,uint16_t len)
{
    uint8_t i = 0;

    num = num;
    i = 0;

    if((NetConfigInfo[NET_YX_SELCT].NetNum > 1) && (SIM7600CmdState.SIM_ConectState[0] == STATE_OK))
    {
        if((num > 10) || (pdata == NULL) )
        {
            return RT_FALSE;
        }
        Send_AT_CIPSEND(num,len);
        while (i < CM_TIME_10_SEC / CM_TIME_50_MSEC)
        {
            rt_thread_mdelay(CM_TIME_50_MSEC);
            //等待AT指令被回复
            if (SIM7600CmdState.ATCMD_SENDACK[num] == _4G_RESPOND_OK)
            {
                SIM7600CmdState.ATCMD_SENDACK[num] = _4G_RESPOND_ERROR;
                break;
            }
            i++;
            if((i % 20) == 0) //1s钟没回复 就继续发送
            {
                Send_AT_CIPSEND(num,len);       //多发比发不出去好！
            }
        }
        if (i == CM_TIME_10_SEC / CM_TIME_50_MSEC)
        {
            APP_SetNetNotConect(num);
            //SIM7600CmdState.SIM_ConectState[num] = STATE_ERR;  //等待2s还没发送数据，则重新连接网络
            rt_kprintf("respond cmd:ATCMD_SENDACK timeout");
            return RT_FALSE;
        }
    }


    //真正发送数据
    if (UART_4GWrite(pdata,len ) != len )
    {
        return RT_FALSE;
    }

    rt_thread_mdelay(CM_TIME_1_SEC);
    return RT_TRUE;

}

/*****************************************************************************
* Function     :ModuleSIM7600_ReadData
* Description  :读取数据请求
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t ModuleSIM7600_ReadData(uint8_t num)
{
    if(num > 10)
    {
        return RT_FALSE;
    }
    Send_AT_CIPRXGET(num);
    return RT_TRUE;
}

/*****************************************************************************
* Function     :SIM7600_RecvDesposeCmd
* Description  :命令模式下模块接收处理
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t SIM7600_RecvDesposeCmd(uint8_t *pdata,uint32_t len)
{
    uint8_t i;

    if((pdata == NULL) || (!len))
    {
        return RT_FALSE;
    }
    for(i = 0; i < DIM7600_RECV_TABLE_LEN; i++)
    {

        if (!strncmp( (char *)pdata, TIM7600_RecvFrameTable[i].ATCode, MIN(strlen(TIM7600_RecvFrameTable[i].ATCode),len) ) )
        {
            TIM7600_RecvFrameTable[i].Fun(pdata,len);
            return RT_TRUE;
        }
    }
    return RT_FALSE;

}

/*****************************************************************************
* Function     :APP_SetNetNotConect
* Description  :设置网络未连接
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t APP_SetNetNotConect(uint8_t num)
{
    if(num >= NetConfigInfo[NET_YX_SELCT].NetNum)
    {
        return RT_FALSE;
    }
    if(NetConfigInfo[NET_YX_SELCT].NetNum == 1)
    {
        SIM7600CmdState.SIM_ExistState = STATE_ERR;   //只有一个模块的时候需要重新启动
        CSQNum = 0;			//信号强度为0
    }
    SIM7600CmdState.SIM_ConectState[num] = STATE_ERR;
    SIM7600CmdState.SIM_Regiset[num] = STATE_ERR;
    SIM7600CmdState.ATCMD_CIPOPEN[num] = STATE_ERR;
    return RT_TRUE;
}


/* Private macro------------------------------------------------------------------------------*/

/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

/************************(C)COPYRIGHT 2018 杭州汇誉*****END OF FILE****************************/

