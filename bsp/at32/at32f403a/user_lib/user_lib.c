/*******************************************************************************
 *          Copyright (c) 2020-2050, .
 *                              All Right Reserved.
 * @file user_lib.c
 * @note 
 * @brief
 * 
 * @author   
 * @date     2020-12-01
 * @version  V1.0.0
 * 
 * @Description 
 *  
 * @note History:        
 * @note     <author>   <time>    <version >   <desc>
 * @note  
 * @warning
 *******************************************************************************/
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "user_lib.h"
#include "timestamp.h"

const uint8_t	gCuc_Hex2Char[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static const uint8_t gCuc_BCD2HexMap[] =
{
//  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //0
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //1
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //2
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //3
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //4
    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //5
    0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //6
    0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //7
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //8
    0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, //0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //9
};

static const uint8_t gCuc_Hex2BCDMap[] =
{
//  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,	//0-9
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,	//10-19
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,	//20-29
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,	//30-39
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,	//40-49
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,	//50-59
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,	//60-69
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,	//70-79
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,	//80-89
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,	//90-99
};

/**
 * @brief �����ֵ����
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
int max_func(int a[],int n)
{
    int i;
	int max;
    for(max=a[0],i=1;i<n;i++)	//��ʼĬ�����ֵΪa[0],��������Ԫ��
    {    
		if(max<a[i])  			//�����ǰ���ֵ<��ǰ����Ԫ�أ��򽫸�Ԫ�ظ�ֵ��max
        {
			max=a[i];
		}
	}
    return max;
}

/**
 * @brief ����Сֵ����
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
int min_func(int a[],int n)
{
    int i;
	int min;
    for(min=a[0],i=1;i<n;i++)	//��ʼĬ����СֵΪa[0],��������Ԫ��
    {   
		if(min>a[i])  			//�����ǰ��Сֵ>��ǰ����Ԫ�أ��򽫸�Ԫ�ظ�ֵ��min
		{
			min=a[i];
		}
	}
    return min;
}

///**
// * @brief bitλת��Ϊֵ
// * @param[in]   
// * @param[out] 
// * @return
// * @note
// */
//uint32_t BitToValue(uint32_t uiBit)
//{
//	uint32_t i;
//	uint32_t value=0;
//	for(i=0;i<32;i++) 
//	{
//		if(BIT(i) & uiBit)
//		{
//			value += pow(2.0,i); 
//		}			
//	} 
//	return value;	
//}

/**
 * @brief ��src��ָ�ַ���ת��Ϊ16�����������dest��ת���ַ���Ϊlen
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void StringToHex(char *dest, char *src, int len)
{
	int i=0;
	int j=0;
	unsigned char temp;
	while(i<len)
	{
		temp=src[i];
		if((temp>=0x30)&&(temp<=0x39))						//0~9
		{
			temp=temp-'0';
			dest[j]=temp<<4;
		}
		else if((temp>=0x41)&&(temp<=0x46))					//A~F
		{
			temp=temp-'A'+0x0A;
			dest[j]=temp<<4;
		}
		else if((temp>=0x61)&&(temp<=0x66))					//a~f
		{
			temp=temp-0x61+0x0A;
			dest[j]=temp<<4;
		}
		else
		{
			dest[j]=0x00;
		}
		temp=src[i+1];										//ÿ������ת�������ַ�
		if((temp>=0x30)&&(temp<=0x39))
		{
			temp=temp-0x30;
			dest[j]=dest[j]|temp;
		}
		else if((temp>=0x41)&&(temp<=0x46))
		{
			temp=temp-0x41+0x0A;
			dest[j]=dest[j]|temp;
		}
		else if((temp>=0x61)&&(temp<=0x66))
		{
			temp=temp-0x61+0x0A;
			dest[j]=dest[j]|temp;
		}
		else
		{
			dest[j]=dest[j]|0x00;
		}
		i=i+2;
		j=j+1;
	}
	return;
}

/**
 * @brief ��uint8_tת��Ϊuint16_t����(С��ģʽ)������
 * @param[in] pucSrc ��ת����uint8_t��������   
 * @param[out] 
 * @return ת�����uint16_t��������
 * @note
 */
uint16_t uint8_to_uint16_SE(uint8_t *pucSrc)
{
	return ((uint16_t)pucSrc[1]<<8) + pucSrc[0];
}

/**
 * @brief ��uint8_tת��Ϊuint16_t����(���ģʽ)������
 * @param[in] pucSrc ��ת����uint8_t��������  
 * @param[out] 
 * @return ת�����uint16_t��������
 * @note
 */
uint16_t LIB_uint8_to_uint16_BE(uint8_t *pucSrc) 
{
	return ((uint16_t)pucSrc[0]<<8) + pucSrc[1];
}

/**
 * @brief ��uint16_t����ת��Ϊuint8_t(С��ģʽ)������
 * @param[in] uiSrc ��ת������  
 * @param[out] pucDst ת�����uint8_t�������ݴ�ŵ�ַ 
 * @return
 * @note
 */
void uint16_to_uint8_SE(uint8_t *pucDst, uint16_t uiSrc)
{
    *pucDst++ = (uint8_t)uiSrc;
    *pucDst = (uint8_t)(uiSrc>>8);
}

/**
 * @brief ��uint16_t���͵����ߵ��ֽڲ�����
 * @param[in] usSrc ��ת������   
 * @param[out]  
 * @return uiret ת���������
 * @note
 */
uint16_t uint16_H_to_L(uint16_t usSrc)
{
	uint16_t usret = 0;
    
    usret = (usSrc&0x00FF);
	usret <<= 8;
	usret |= (uint8_t)(usSrc>>8);
	return usret;	
}

/**
 * @brief ��uint32_t���͵����ֽں͸����ֽڷֱ�����ߵ��ֽڣ�Ȼ�󽫵����ֽں͸����ֽڵ����ߵ��ֽڣ��õ�һ���µ�uint32_t������
 * @param[in] uiSrc ��ת������   
 * @param[out] 
 * @return uiret ת���������
 * @note ����0x00100100-->0x10000001-->0x00011000
 */
uint32_t uint32split_H_to_L(uint32_t uiSrc)
{
	uint16_t Hdata;
	uint16_t Ldata;
	uint32_t uiret = 0;
	
	
	Ldata = (uint16_t)(uiSrc & 0xFFFF);
	Ldata = uint16_H_to_L( Ldata );
	
	Hdata = (uint16_t)(uiSrc >> 16);
	Hdata = uint16_H_to_L( Hdata );
	
	uiret = (Ldata << 16) | Hdata;
	
	return uiret;
	
}

/**
 * @brief ��uint16_t����ת��Ϊuint8_t(���ģʽ)������
 * @param[in] uiSrc ��ת������   
 * @param[out] pucDst ת�����uint8_t�������ݴ�ŵ�ַ
 * @return
 * @note
 */
void uint16_to_uint8_BE(uint8_t *pucDst, uint16_t uiSrc)
{
    *pucDst++ = (uint8_t)(uiSrc>>8);
    *pucDst = (uint8_t)uiSrc;
}

/**
 * @brief ��uint32_t����ת��Ϊuint8_t(С��ģʽ)������
 * @param[in] ulSrc ��ת������   
 * @param[out] pucDst ת�����uint8_t�������ݴ�ŵ�ַ 
 * @return
 * @note
 */
void uint32_to_uint8_SE(uint8_t *pucDst, uint32_t ulSrc)
{
    uint8_t *p = (uint8_t *)&ulSrc;

    *pucDst++ = *p++;
    *pucDst++ = *p++;
    *pucDst++ = *p++;
    *pucDst = *p;
}

/**
 * @brief ��uint32_t����ת��Ϊuint8_t(���ģʽ)������
 * @param[in] uiSrc ��ת������   
 * @param[out] pucDst ת�����uint8_t�������ݴ�ŵ�ַ 
 * @return 
 * @note
 */
void uint32_to_uint8_BE(uint8_t *pucDst, uint32_t uiSrc)
{
    uint8_t *p = (uint8_t *)&uiSrc + 3;

    *pucDst++ = *p--;
    *pucDst++ = *p--;
    *pucDst++ = *p--;
    *pucDst = *p;
}

/**
 * @brief ��uint8_t����ת��Ϊ24λ��������(С��ģʽ)������
 * @param[in] pucSrc ת��ǰ��uint8_t�������ݴ�ŵ�ַ  
 * @param[out] ת�����uint32_t�������� 
 * @return
 * @note
 */
uint32_t LIB_uint8_to_uint24_SE(uint8_t *pucSrc)
{
    uint8_t *p;
    uint32_t ulRet = 0;

    p = (uint8_t *)&ulRet;
    *p++ = *pucSrc++;
    *p++ = *pucSrc++;
    *p = *pucSrc;

    return ulRet;
}

/**
 * @brief ��uint8_t����ת��Ϊuint32_t(С��ģʽ)������
 * @param[in] pucSrc:ת��ǰ��uint8_t�������ݴ�ŵ�ַ  
 * @param[out] ת�����uint32_t�������� 
 * @return
 * @note
 */
uint32_t LIB_uint8_to_uint32_SE(uint8_t *pucSrc)
{
    uint8_t *p;
    uint32_t ulRet;

    p = (uint8_t *)&ulRet;
    *p++ = *pucSrc++;
    *p++ = *pucSrc++;
    *p++ = *pucSrc++;
    *p = *pucSrc;

    return ulRet;
}

/**
 * @brief ��uint8_t����ת��Ϊuint32_t(���ģʽ)������
 * @param[in] pucSrc ת��ǰ��uint8_t�������ݴ�ŵ�ַ   
 * @param[out]  
 * @return ת�����uint32_t��������
 * @note
 */
uint32_t LIB_uint8_to_uint32_BE(uint8_t *pucSrc)
{
    uint8_t *p;
    uint32_t ulRet;

    p = (uint8_t *)&ulRet + 3;
    *p-- = *pucSrc++;
    *p-- = *pucSrc++;
    *p-- = *pucSrc++;
    *p = *pucSrc;

    return ulRet;
}

/**
 * @brief ��uint16_t���͵�����������иߵ��ֽڵ����󸳸�pucDst��ָ����(���ģʽ)
 * @param[in] uiSrc ��ת������  
 * @param[out] pucDst ת�����uint8_t�������ݴ�ŵ�ַ 
 * @return
 * @note
 */
void LIB_uint16_to_uint8p_BE(uint8_t *pucDst, uint16_t uiSrc)
{
    *pucDst++ = (uint8_t)(uiSrc>>8);
    *pucDst = (uint8_t)uiSrc;
}

/**
 * @brief ��uint16_t���͵�����������иߵ��ֽڵ����󸳸�pucDst��ָ����
 * @param[in] uiSrc ��ת������  
 * @param[out] pucDst ת�����uint8_t�������ݴ�ŵ�ַ 
 * @return
 * @note
 */
void LIB_uint16_to_uint8p(uint8_t* pucDst, uint16_t uiSrc)
{
    *pucDst++ = (uint8_t)uiSrc;
    *pucDst = (uint8_t)(uiSrc>>8);
}

/**
 * @brief 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
uint16_t LIB_uint8p_to_uint16(uint8_t* pucSrc)
{
	return ((uint16_t)pucSrc[1]<<8) + pucSrc[0];
}

/**
 * @brief 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
uint8_t LIB_CheckSumByte(uint8_t* p, uint32_t ulLen)
{
    uint8_t ucSum = 0;

    while(ulLen--)
    {
        ucSum += (*p++);
    }
    
    return ucSum;
}

/**
 * @brief 
 * @param[in] ucBCD:Դ����,BCD type  
 * @param[out] 
 * @return uint8_t :Ŀ������,hex type
 * @note �������������򷵻�0xAA
 */
uint8_t LIB_UcharBCD2Hex(uint8_t ucBCD)
{
    if(ucBCD > 0x99)
    {
        return 0xAA;
    }
    return gCuc_BCD2HexMap[ucBCD];
}

/**
 * @brief 
 * @param[in] uiBCD:Դ����,BCD type  
 * @param[out] 
 * @return
 * @note
 */
uint16_t LIB_UshortBCD2Hex(uint16_t uiBCD)
{
	uint8_t i;
    uint16_t uiRet = 0;
    uint8_t* p = (uint8_t*)&uiBCD + 1;
    for(i=0; i<2; i++)
    {
        uiRet *= 100;
        uiRet += LIB_UcharBCD2Hex(*p--);
    }
    return uiRet;
}

/**
 * @brief 24λUlong����BCD��ת��ΪHex���ݸ�ʽ
 * @param[in] ulBCD:Դ����,BCD type  
 * @param[out] 
 * @return 24λUlong����HEX����
 * @note
 */
uint32_t LIB_U24BitBCD2Hex(uint32_t ulBCD)
{
    uint8_t i;
    uint32_t ulRet = 0;
    uint8_t* p = (uint8_t*)&ulBCD + 2;

    for(i=0; i<3; i++)
    {
        ulRet *= 100;
        ulRet += LIB_UcharBCD2Hex(*p--);
    }

    return ulRet;
}

/**
 * @brief Ulong����BCD��ת��ΪHex���ݸ�ʽ
 * @param[in] uint32_t Դ����,BCD type  
 * @param[out] 
 * @return uint32_t Ŀ������,hex type
 * @note
 */
uint32_t LIB_UlongBCD2Hex(uint32_t ulBCD)
{
    uint8_t i;
    uint32_t ulRet = 0;
    uint8_t* p = (uint8_t*)&ulBCD + 3;

    for(i=0; i<4; i++)
    {
        ulRet *= 100;
        ulRet += LIB_UcharBCD2Hex(*p--);
    }

    return ulRet;
}

/**
 * @brief ʮ����תBCD��
 * @param[in] Dec:��ת����ʮ��������   length:BCD�����ݳ���  
 * @param[out] Bcd:ת�����BCD��
 * @return
 * @note
 */
void LIB_U32DectoBCD(uint32_t Dec, uint8_t *Bcd, uint32_t length)
{
     int i;
     int temp;

     for(i=length-1; i>=0; i--)
     {
         temp = Dec%100;
         Bcd[i] = ((temp/10)<<4) + ((temp%10) & 0x0F);
         Dec /= 100;
     }
}

/**
 * @brief ʮ����תBCD��
 * @param[in] Dec:��ת����ʮ��������   length:BCD�����ݳ���  
 * @param[out] Bcd:ת�����BCD��
 * @return
 * @note
 */
void LIB_U64DectoBCD(uint64_t Dec, uint8_t *Bcd, uint32_t length)
{
     int i;
     int temp;

     for(i=length-1; i>=0; i--)
     {
         temp = Dec%100;
         Bcd[i] = ((temp/10)<<4) + ((temp%10) & 0x0F);
         Dec /= 100;
     }
}

/**
 * @brief ASC��ת��ΪBCD���ݸ�ʽ() 
 * @param[in] input:Դ����(ASC type)  inputLen:�����������   
 * @param[out] output:ת����Ĵ�Ż��� 
 * @return 0:����; 1:��ȷ
 * @note ������39 66 65 39 34 35 61 37 61 32 33 62 33 63 65 39 36 38 31 34 33 31 63 33 65 33 31 31 33 34 65 65
 *       (ת�������9fe945a7a23b3ce9681431c3e31134ee)
 */
uint8_t LIB_Asc2Bcd(uint8_t *input, uint32_t inputLen, uint8_t *output)
{
	uint8_t temp;
	uint32_t i;

    if (input == NULL || output == NULL) 
    {
		return 0;
	}

	for (i = 0; i < inputLen; i += 2) 
	{
		temp = input[i];
		if (temp > '9') 
		{
			temp = (unsigned char)toupper((unsigned char)temp) - 'A' + 0x0A;
		}
		else 
		{
			temp &= 0x0F;
		}
		
		output[i / 2] = temp << 4;

		temp = input[i+1];
		if (temp > '9') 
		{
			temp = (unsigned char)toupper((unsigned char)temp) - 'A' + 0x0A;
		}
		else 
		{
			temp &= 0x0F;
		}
		
		output[i/2] |= temp;
	}
    return 1;
}

/**
 * @brief BCD��ת��ΪASC���ݸ�ʽ
 * @param[in] input:Դ����(BCD type)    inputLen:�����������  
 * @param[out] output:ת����Ĵ�Ż���
 * @return 0:����; 1:��ȷ
 * @note
 */
uint8_t LIB_Bcd2Asc(uint8_t *input, uint32_t inputLen, uint8_t *output)
{
    const unsigned char ascii[] = "0123456789ABCDEF";
    unsigned int i;

    if (input == NULL || output == NULL)
	{
		return 0;
	}
    for (i = 0; i < inputLen; i++)
    {
        output[2 * i]   = ascii[(input[i] >> 4)];
        output[2 * i + 1] = ascii[(input[i] & 0x0F)];
    }
    return 1;
}

/**
 * @brief 
 * @param[in] psClkָ��ʱ�ӱ������׵�ַ  
 * @param[out] ʱ�ӱ��������Ĵ�2000��1��1����������� 
 * @return
 * @note hhmmss���������,������YYMMDD���ۼ�����,��2000��1��1��Ϊ��׼�����ڵ���������,���ʱ�䲻����2000��֮ǰ
 *       (��ʵ2000����00��ʾ,�������϶���ȥ)
 */
static const uint16_t gui_MonthBeginDay[] = {0, 31, 59, 90,120, 151, 181, 212, 243, 273, 304, 334};// ÿ����ʼ���������
uint16_t LIB_CalcAbsDay(S_API_TIME* psClk)
{
    uint8_t ucYear = psClk->ucYear;        	                //��,Hex,0-255
    uint8_t ucMonth = psClk->ucMonth;		                //��,Hex,0-11
    uint8_t ucDay = psClk->ucDay;	                        //��,Hex,0-30

    if((ucYear & 0x03) == 0)                            	//�ж��Ƿ���������2��
    {
        if(ucMonth > 1)
		{
			ucDay++;                                    	//�ټ�һ��
		}
    }
    else
    {
        ucDay++;                                        	//���������00�����¶������һ��
    }
    														//year >> 2,�Ǽ��Ϲ�ȥ����4��������һ��
    return ((uint16_t)ucYear*365 + gui_MonthBeginDay[ucMonth] + (ucDay + (ucYear>>2)));
}

/**
 * @brief 
 * @param[in] psClkָ��ʱ�ӱ������׵�ַ   
 * @param[out] 
 * @return ʱ�ӱ���������hhmmss����������������
 * @note YYMMDD���������,������hhmmss���ۼ����,�Ե���00ʱ00��00��Ϊ��׼��
 */
uint32_t LIB_CalcSecondOfThisDay(S_API_TIME* psClk)
{
    uint8_t ucSec =psClk->ucSec + 1;	    	            //��,Hex,0-59
    uint8_t ucMin = psClk->ucMin + 1;	    	            //��,Hex,0-59
    uint8_t ucHour = psClk->ucHour + 1;	    	        	//ʱ,Hex,0-23

    return (ucSec + (uint16_t)ucMin*60 + (uint32_t)ucHour*3600);
}

/**
 * @brief 
 * @param[in] psClk1,psClk2ָ��ʱ�ӱ������׵�ַ  
 * @param[out] 
 * @return ͬһ�������е���ֵ֮��ľ���ֵ
 * @note
 */
uint32_t LIB_CalcDaySecondGap(S_API_TIME* psClk1, S_API_TIME* psClk2)
{
	int lGap = (int)LIB_CalcSecondOfThisDay(psClk1) - (int)LIB_CalcSecondOfThisDay(psClk2);
	if(lGap < 0)
	{
		lGap = -lGap;
	}
	return (uint32_t)lGap;
}

/**
 * @brief psClkָ��ʱ�ӱ������׵�ַ
 * @param[in]   
 * @param[out] ʱ�ӱ������������е���ֵ
 * @return
 * @note ��2000��1��1��Ϊ��׼�����ڵ����е���ֵ
 */
uint32_t LIB_CalcAbsSecond(S_API_TIME* psClk)
{
	return (LIB_CalcAbsDay(psClk)*(3600*24uL) + LIB_CalcSecondOfThisDay(psClk));
}

/**
 * @brief ��ʱ�����ֵת��ΪRTCʱ��
 * @param[in] timestamp:ָ��ʱ���(����)��ַ   
 * @param[out] 
 * @return S_API_TIMEʱ�Ӹ�ʽ
 * @note
 */
SYSTEM_RTCTIME *LIB_TimestampsToRtc(uint32_t timestamp)
{
	S_STAMP rtc;
	static SYSTEM_RTCTIME s_rtc;
	gmtime_new(&rtc,timestamp);
	memcpy((char *)&s_rtc,(char *)&rtc,3);					//��ֵ:��/��/ʱ/					
	memcpy((char *)&s_rtc.ucDay,(char *)&rtc.tm_mday,3);		//��ֵ:��/��/��
	s_rtc.ucWeek += 1;										//��ֵ:����
	s_rtc.ucYear -= 100;
	s_rtc.ucMonth += 1;										//UTC���·ݷ�Χ0~11������Ҫ��1
	return (SYSTEM_RTCTIME *)(&s_rtc);
}

/**
 * @brief ��RTCʱ��ת��Ϊʱ���(��)
 * @param[in] pRtc:ָ��SYSTEM_RTCTIME��ʽʱ���ַ  
 * @param[out] 
 * @return ʱ���(��)
 * @note DS3231ʱ��ֵ��Χ:second:0~59  minute:0~59  hour:0~23  day:1~7    date:0~31  month:1~12  year:0~99
 *       time.h �е�����1900Ϊ������������2018�����ֵΪ118
 */
uint32_t LIB_RtcToTimestamps(SYSTEM_RTCTIME *pRtc)
{
	S_STAMP stamp;
	uint32_t uiStamp;
	memset((char *)&stamp,0,sizeof(stamp));
	memcpy((char *)&stamp,(char *)pRtc,3);						//��ֵ:��/��/ʱ/����
	memcpy((char *)&stamp.tm_mday,(char *)&(pRtc->ucDay),3);	//��ֵ:��/��/��
	stamp.tm_wday = pRtc->ucWeek-1;		 						//��ֵ����:	0~6
	stamp.tm_year += 100;										//�����100
	stamp.tm_mon -= 1;											//0~11
	stamp.tm_yday = gui_MonthBeginDay[stamp.tm_mon]+pRtc->ucDay;	//days since January 1, 0 to 365
	uiStamp =mktime_new(&stamp);								//����time.h�еĿ⺯��(mktime(&rt);)������

	return uiStamp;
}

/**
 * @brief ��UTCʱ��ת��Ϊ����ʱ��(+8)
 * @param[in] pRtc:ָ��ʵʱʱ��  
 * @param[out] 
 * @return RTCʱ��
 * @note DS3231ʱ��ֵ��Χ:second:0~59  minute:0~59  hour:0~23  day:1~7    date:0~31  month:1~12  year:0~99
 *       time.h �е�����1900Ϊ������������2018�����ֵΪ118
 */
SYSTEM_RTCTIME *LIB_UtcToRtc(SYSTEM_RTCTIME *pRtc)
{
	SYSTEM_RTCTIME *p_rtc;
	uint32_t time_cnt;
	time_cnt = LIB_RtcToTimestamps(pRtc);
	time_cnt += 8*3600;								//����ʱ���ڱ�׼UTCʱ���ϼ�8Сʱ
	p_rtc = LIB_TimestampsToRtc(time_cnt);
	return p_rtc;
}

/**
 * @brief 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
SYSTEM_RTCTIME *LIB_RtcToUtc(SYSTEM_RTCTIME *pRtc)
{
	SYSTEM_RTCTIME *p_rtc;
	uint32_t time_cnt;
	time_cnt = LIB_RtcToTimestamps(pRtc);
	time_cnt -= 8*3600;								//��׼UTCʱ���ڱ���ʱ���ϼ�8Сʱ
	p_rtc = LIB_TimestampsToRtc(time_cnt);
	return p_rtc;
}

/**
 * @brief ��UTCʱ��ת��ΪӦ��ʱ��(+8)����ֻ���������գ�ʱ����
 * @param[in] pRtc:ָ��ʵʱʱ��  
 * @param[out] pApi:ָ��ת����Ĵ洢��ַ(APIʱ��,˳��:���ʱ������) 
 * @return
 * @note
 */
void LIB_UtcToApiTime(SYSTEM_RTCTIME *pRtc,S_API_TIME *pApi)
{
	SYSTEM_RTCTIME *p_rtc;
	uint32_t time_cnt;
	time_cnt = LIB_RtcToTimestamps(pRtc);
	time_cnt += 8*3600;											//����ʱ���ڱ�׼UTCʱ���ϼ�8Сʱ
	p_rtc = LIB_TimestampsToRtc(time_cnt);
	memcpy((char *)&pApi->ucSec,(char *)&p_rtc->ucSec,3);		//���ʱ
	memcpy((char *)&(pApi->ucDay),(char *)&p_rtc->ucDay,3);		//������
}

/**
 * @brief �Ƚ�����ʱ��֮������ֵ
 * @param[in] psClk1,psClk2ָ��ʱ�ӱ������׵�ַ  
 * @param[out] 
 * @return ����ʱ�ӱ������������е���ֵ֮��ľ���ֵ
 * @note
 */
int LIB_CalcTimeGap(S_API_TIME* psClk1, S_API_TIME* psClk2)
{
	int lGap = (int)LIB_CalcAbsSecond(psClk1) - (int)LIB_CalcAbsSecond(psClk2);
	if(lGap < 0)
	{
		lGap = -lGap;
	}
	return lGap;
}

/**
 * @brief ��ȡUTCʱ��,��ת����16����
 * @param[in] pt:UTCʱ���ʽ�ַ���   
 * @param[out] 
 * @return 0��ʽ����; ��0��ȡ�ɹ�
 * @note
 */
uint8_t GetUtcTime(char *pt,SYSTEM_RTCTIME *pTime)
{
	char *p=pt;
	pTime->ucYear = atoi(p)%100;  			//��
	if((p = strchr(p,'-')) == NULL)
	{
		return 0;
	}
	pTime->ucMonth = atoi(p+1);				//��
	if((p = strchr(p+1,'-')) == NULL)
	{
		return 0;
	}
	pTime->ucDay = atoi(p+1);				//��
	
//	pTime->week  = 1;						//����ֵоƬ���Զ�����
	
	if((p = strchr(p+1,'T')) == NULL)
	{
		return 0;
	}
	pTime->ucHour = atoi(p+1);				//ʱ
	if((p = strchr(p+1,':')) == NULL)
	{
		return 0;
	}
	pTime->ucMin = atoi(p+1);				//��
	if((p = strchr(p+1,':')) == NULL)
	{
		return 0;
	}
	pTime->ucSec = atoi(p+1);				//��
	return 1;
}

/**
 * @brief ʱ��HEXתASCII,��ʽ�����ʱ������
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
char *GetTimeAscii(char *pTime)
{
	static uint8_t	ucBuf[16];		
	memset((char *)&ucBuf,'\0',sizeof(ucBuf));
							
	sprintf((char *)&ucBuf,"%02x%02x%02x%02x%02x%02x",pTime[0],pTime[1],pTime[2],pTime[3],pTime[4],pTime[5]);
    return (char *)&ucBuf;
}

/**
 * @brief  
 * @param[in] ucHex:Դ����,hex type   
 * @param[out] uint8 :Ŀ������,BCD type 
 * @return
 * @note ����99�򷵻�99
 */
uint8_t LIB_UcharHex2BCD(uint8_t ucHex)
{
    if(ucHex > 99)
    {
        return 0x99;
    }
    return gCuc_Hex2BCDMap[ucHex];
}

/**
 * @brief BCDתʮ����
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
uint8_t BcdToDec(uint8_t Bcd_Data)
{
    uint8_t Dec_Data;
    Dec_Data = ((Bcd_Data>>4)&0x0F)*10 + (Bcd_Data&0x0F);
    return Dec_Data;
}

/**
 * @brief ʮ����תBCD
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
uint8_t DecToBcd(uint8_t DecData)
{
    uint8_t BcdData;
	BcdData = ((DecData/10)<<4) + ((DecData%10) & 0x0F);
    return BcdData;
}

/**
 * @brief 
 * @param[in] uint16:Դ����,hex type  
 * @param[out] 
 * @return uint16 :Ŀ������,BCD type
 * @note ����ģ���100�ķ���,�Ƚ��ʺ���û��Ӳ����������cpu
 */
uint16_t LIB_UintHex2BCD(uint16_t uiHex)
{
	uint16_t uiRet = 0;

	if(uiHex > 9999)
	{
		return 0x9999;
	}

	if(uiHex >= 8000)
	{
		uiRet |= 0x8000;
		uiHex -= 8000;
	}

	if(uiHex >= 4000)
	{
		uiRet |= 0x4000;
		uiHex -= 4000;
	}

	if(uiHex >= 2000)
	{
		uiRet |= 0x2000;
		uiHex -= 2000;
	}

	if(uiHex >= 1000)
	{
		uiRet |= 0x1000;
		uiHex -= 1000;
	}

	if(uiHex >= 800)
	{
		uiRet |= 0x800;
		uiHex -= 800;
	}

	if(uiHex >= 400)
	{
		uiRet |= 0x400;
		uiHex -= 400;
	}

	if(uiHex >= 200)
	{
		uiRet |= 0x200;
		uiHex -= 200;
	}

	if(uiHex >= 100)
	{
		uiRet |= 0x100;
		uiHex -= 100;
	}

	uiRet |= LIB_UcharHex2BCD(uiHex);
	
	return uiRet;
}

/**
 * @brief �ڴ濽��
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void my_memcpy(char *dst, char *src, uint16_t uiLen)
{
	uint16_t i;
	for(i = 0;i<uiLen;i++)
	{
		*dst++ = *src++;
	}
}

/**
 * @brief �ڴ����򿽱�,�ӿ���Դsrc���һ��Ԫ����ǰ������dstָ���Ļ��� 
 * @param[in]   
 * @param[out] 
 * @return
 * @note ���ã� ֻ�ʺϴ��ģʽ�����򿽱�
 */
void memcpy_reverse(char *dst, char *src, uint16_t uiLen)
{
	uint16_t i;
	
	char *p_src = src+uiLen-1;		//ָ�򿽱�Դ�Ķ���
	
	for(i = 0;i<uiLen;i++)
	{
		*dst++ = *p_src--;
	}
}

/**
 * @brief �ڴ������ƶ�,��offset����n��Ԫ����ǰ������dstָ���Ļ��棬Ȼ�������ƶ�ʣ������ݣ��ƶ����׵�ַ(�������ӵ��е���ԭ��)
 * @param[in] src:ָ���ڴ��׵�ַ; dst:ָ��Ŀ�Ĵ�(Ҳ�ڵ�ǰ�ڴ���); usTotalLen:Ԫ�����ܳ���;usLen��ת�����ݵĳ���  
 * @param[out] 
 * @return ʣ������ݳ���
 * @note ԭ�ڴ�����Ϊ{0,1 2 3 4 5 6 7 8 9},��0~5�����ݿ��ߺ��򿼱�����ڴ�Ϊ��{6 7 8 9}
 */
uint16_t memremove(char *src, char *dst, uint16_t usTotalLen,uint16_t usLen)
{
	uint16_t i;
	
	if(usTotalLen < usLen)
	{
		memcpy(dst,src,usTotalLen);	//��������
		return 0;
	}
	
	memcpy(dst,src,usLen);
	
	for(i=0; i<usTotalLen-usLen; i++)
	{
		src[i] = src[usLen + i];
	}
	return (usTotalLen-usLen);
}

/**
 * @brief ���ַ�ת����Сд��ĸ,����ĸ��Сд��ĸ����������
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
int tolower(int c)   
{   
	if (c >= 'A' && c <= 'Z')   
	{   
		return c + 'a' - 'A';   
	}   
	else   
	{   
		return c;   
	}   
} 

/**
 * @brief �������ַ�תƴ�ճɶ�Ӧ��16������
 * @param[in]   
 * @param[out] 
 * @return
 * @note �ַ���"bc"ת����0xbc
 */
uint8_t Asc2Hex( uint8_t *p)
{
	uint8_t ch;
	LIB_Asc2Bcd(p, 2, &ch);
	return ch;
}

/**
 * @brief 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
int htoi(const char s[],int start,int len)   
{   
	int i; 
	int j;
	int n = 0;
	if (s[0] == '0' && (s[1]=='x' || s[1]=='X'))
	{   
		i = 2;   
	}   
	else   
	{   
		i = 0;   
	}   
	i+=start;  
	j=0;  
	for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f') || (s[i] >='A' && s[i] <= 'F');++i)   
	{     
		if(j>=len)  
		{  
			break;  
		}  
		if (tolower(s[i]) > '9')   
		{   
			n = 16 * n + (10 + tolower(s[i]) - 'a');   
		}   
		else   
		{   
			n = 16 * n + (tolower(s[i]) - '0');   
		}   
		j++;  
	}   
	return n;   
}

/**
 * @brief ��s1��ǰlen���ַ����в���s2��������ھͷ���s2��s1�е�λ�ã����û���ҵ�������NULL
 * @param[in] s1:ָ��Դ�ַ���; s2:ָ�����ַ���; len:Դ�ַ�������  
 * @param[out] 
 * @return
 * @note ���s2�ǵ����ַ�,ҲҪ��""������������'',��"U"��
 *       strstr�������ڱ����ҵ��ַ���֮ǰ��0x00�Ļ��ͻ�����,������,��˽�����strnstr()
 *       �ú���ʹ��ǰ����s1��s2������'\0'��β,�����޷������s1��s2�ĳ���
 */
char *strnstr(char *s1, char *s2, int len)
{
	char *p;
	int len1;
	int len2;
//	len1 = strlen(s1);		//�����������,���Դ�ַ���������'\0'��ʼ���򳤶�Ϊ0���޷�����������
	len1 = len;
	len2 = strlen(s2);
	p = s1;
	
	if (!len2 || !len1)
	{
        return NULL;
    }		
	
	if(len2>len1)
	{
        return NULL;
    }
	
	while (len1 >= len2) 
	{
		len1--;
		if (memcmp(p, s2, len2)==0)
		{
            return (char *)p;
        }
		p++;
	}
	return NULL;
}

/**
 * @brief ��s1�ַ����в���s2�����ҷ�ΧΪlen�ֽڳ��ȣ�������ھͷ���s2��s1�е������ֵ�λ�ã�
 *        ���û���ҵ������� NULL��
 * @param[in] s1-ָ��Դ�ַ���; s2-ָ�����ַ���; len-Դ�ַ�������  
 * @param[out] 
 * @return
 * @note ���s2�ǵ����ַ���ҲҪ��""������������''����"U"��
 *       �ú���ʹ��ǰ����s1��s2������'\0'����β�������޷������s1��s2�ĳ���
 */
char* strrnstr(char* s1, char* s2, int len)
{
	char *p,*pFind = NULL;
	int len1,len2;
	
	len1 = len;
	len2 = strlen(s2);
	p = s1;
	if (!len2 || !len1)
		return NULL;		
	
	if(len2>len1)
		return NULL;
	while (len1 >= len2) 
	{
		if (memcmp(p, s2, len2)==0)
			pFind = p;
		len1 = len1 - len2;
		p += len2;
	}
	return pFind;
}

/**
 * @brief �ַ�����ȡ���ܣ������������з�(\r��\n����һ��)������
 * @param[in] src:����ȡ���ַ�Դ    uslen:Դ�ַ����ܳ���  
 * @param[out] dst:ָ����ȡ���ŵĻ���
 * @return len:��ȡ���ַ�������
 * @note
 */
uint16_t GetStrstr(char *src, char *dst, uint16_t uslen)
{
	uint16_t i=uslen;
	uint16_t len=0;
	char *pstart;
	
	while(i)			 //Ѱ�ҵ�һ����Ч�ַ�,��һ���ַ����������ֻ���ĸ��ʼ
	{
		if((*src>='0' && *src<='9') || (*src>='a' && *src<='z') || (*src>='A' && *src<='Z'))
		{
			break;
		}
		src++;
		i--;
	}
	
	if((i == 0) || (i > uslen))
	{
		return 0;
	}
	
	pstart = src;
	while(i--)
	{
		len++;
		src++;
		if(*src=='\r' || *src=='\n')
		{
			break;
		}
	}	
	memcpy(dst, pstart, len);
	return len;
}

/**
 * @brief ʮ������תASCII��(���֧��9λ��)
 * @param[in] dec:ʮ����Դ����   
 * @param[out] dst:ת������λ��
 * @return ת������ַ���
 * @note
 */
uint8_t LIB_Dec2Ascii(uint32_t dec, char *dst)
{
	uint8_t i=0;
	uint8_t tmp[10];

	if(dec == 0)
	{
		return 0;
	}
	while(1)
	{
		tmp[i++] = (dec%10) + '0';
		dec = dec/10;
		if(dec == 0)
		{
			break;
		}			
		if(i >= 10)
		{
			break;
		}
	}
	memcpy_reverse(dst, (char *)tmp, i);
	return i;
}

/**
 * @brief ASCII��תʮ��������ת(���֧��8���ַ�(4�ֽ�)) 
 * @param[in] src:��ת�����ַ���  src_len:��ת���ĳ���   
 * @param[out] dst:ת������λ�� 
 * @return 1ת���ɹ� 0ת��ʧ�ܻ����ݲ����Ϲ淶
 * @note
 */
char atox(char *src,char *dst,uint16_t src_len)
{
	uint8_t i=0;
	uint16_t len = 0;
	*dst = 0;
	
	while(*src)
	{
		if(((*src>='0') && (*src<='9')) || ((*src>='a') && (*src<='f')) || ((*src>='A') && (*src<='F')))
		{
			*dst <<= 4; 
			if((*src>='0') && (*src<='9'))
			{
				*dst |= *src-'0';
			}
			else if((*src>='a') && (*src<='z'))
			{
				*dst |= *src-'a' + 10;
			}
			else 		
			{
				*dst |= *src-'A' + 10;
			}
			i++;
			if(i>7)						//���֧��8���ַ�(4�ֽ�)
			{
				break;
			}
		}
		else 
		{
			break;
		}
		src++;
		len++;
		if(len >= src_len)
		{
			break;
		}
	}
	return i;	
}

/**
 * @brief ��pucSrc��ָ��hex����ת����ascii���͵������pucDstָ��Ļ���
 * @param[in] pucSrc:Դ���ݴ����� 
 * @param[in] ucLen:��Ҫת����hex���ݳ���
 * @param[out] pucDst:Ŀ�Ļ���,������3����ucLen���� 
 * @return 1.����pucSrc��ŵĸ�ʽΪ: A1,ת����pucDst�еĸ�ʽΪ'A', '1'
 * @note
 */
void LIB_Hex2Ascii(uint8_t *pucDst, uint8_t *pucSrc, uint8_t ucLen)
{
    uint8_t ucTmp;
	
    while(ucLen > 0)
    {
        ucLen--;
        ucTmp = (*pucSrc >> 4);
        if(ucTmp > 9)
        {
            *pucDst = ucTmp + 'A' - 10;
        }
        else
        {
            *pucDst = ucTmp + '0';
        }
        pucDst++;
        
        ucTmp = *pucSrc++ & 0x0F;
        if(ucTmp > 9)
        {
            *pucDst = ucTmp + 'A' - 10;
        }
        else
        {
            *pucDst = ucTmp + '0';
        }
    }
}

/**
 * @brief ��pucSrc��ָ��hex���ݴ�ת����ascii���͵����ݴ��pucDstָ��Ļ���,pucSrc�ֽ�֮����Ҫ��һ���ո�
 * @param[in] pucSrc:Դ���ݴ�����
 * @param[in] ucLen:��Ҫת����hex���ݳ���
 * @param[out] pucDst:Ŀ�Ļ���,������3����ucLen���� 
 * @return
 * @note 1.����pucSrc��ŵĸ�ʽΪ: A1 23 5c d0(��ַ�ӵ͵���),ת����pucDst�еĸ�ʽΪ
 *       'A', '1', ' ' '2', '3', ' ' '5', 'C', ' ' 'D', '0'(��ַ�ӵ͵���)
 */
void LIB_Hex2Ascii_Blank(uint8_t *pucDst, uint8_t *pucSrc, uint8_t ucLen)
{
    uint8_t ucTmp;
	
    while(ucLen > 0)
    {
        ucLen--;
        ucTmp = (*pucSrc >> 4);
        if(ucTmp > 9)
        {
            *pucDst = ucTmp + 'A' - 10;
        }
        else
        {
            *pucDst = ucTmp + '0';
        }
        pucDst++;
        
        ucTmp = *pucSrc++ & 0x0F;
        if(ucTmp > 9)
        {
            *pucDst = ucTmp + 'A' - 10;
        }
        else
        {
            *pucDst = ucTmp + '0';
        }
        pucDst++;
        
        *pucDst++ = ' ';
    }
}

/**
 * @brief 
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
uint16_t Str_ConvertTo_Dec(const char *str)
{
	uint8_t i = 0;
	uint8_t tmp;
	uint16_t num_hex,hex_mod;
	uint8_t len = strlen(str);		/*string length*/

	num_hex = 0;
	hex_mod = 1;
	for (i = 1; i <= len; i++)
	{
		if ((str[len - i] >= '0' && str[len - i] <= '9'))
		{
			tmp = str[len - i] - '0';
			num_hex += tmp*hex_mod;
			hex_mod *= 10;
		}
		else
		{
			num_hex = 0;
		}
	}
	return num_hex;
}

/**
 * @brief ����������ת�����ַ���
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
char *itoa(int value, char *string, int radix)
{
	int     i, d;
	int     flag = 0;
	char    *ptr = string;

	/* This implementation only works for decimal numbers. */
	if (radix != 10)
	{
		*ptr = 0;
		return string;
	}

	if (!value)
	{
		*ptr++ = 0x30;
		*ptr = 0;
		return string;
	}

	/* if this is a negative value insert the minus sign. */
	if (value < 0)
	{
		*ptr++ = '-';

		/* Make the value positive. */
		value *= -1;
		
	}

	for (i = 10000; i > 0; i /= 10)
	{
		d = value / i;

		if (d || flag)
		{
			*ptr++ = (char)(d + 0x30);
			value -= (d * i);
			flag = 1;
		}
	}

	/* Null terminate the string. */
	*ptr = 0;

	return string;

} /* NCL_Itoa */

/**
 * @brief 
 * @param[in]   
 * @param[out]   
 * @return
 * @note
 */
uint32_t FloatToUint32(float *pdata)
{
	int8_t *p;
	int8_t d1,d2,d3,d4;
	uint32_t Result;
	
	p = (int8_t *)pdata;
	d1 = *p;//gd32ΪС��ģʽ�����ֽ����ڴ��λ��
	d2 = *(p+1);
	d3 = *(p+2);
	d4 = *(p+3);
	
	Result = (d4 << 24) | (d3 << 16) | (d2 << 0) | d1;
	
	return Result;

}


/**
 * @brief CRCУ��
 * @param[in] ptr--У������ָ��
 * @param[in] len--У�����ݳ���
 * @param[out]   
 * @return CRCУ���룬˫�ֽ�
 * @note
 */
uint16_t ModbusCRC16(uint8_t *ptr, uint16_t len)
{
    uint8_t i;
    uint16_t crc = ~0x00;

    if((ptr == NULL) || (len == 0xFFFF)) return crc;

    while(len--)
    {
        crc ^= *ptr++;
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x01)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return(crc);
}

/**
 * @brief ��ȡ���������еķ�0ֵ,������Щ��0ֵ���浽�µ�������
 * @param[in] 
 * @param[in] 
 * @param[out]   
 * @return 
 * @note
 */
uint16_t StrTrim(char *pStr, char *pDst, uint16_t len)
{
	uint16_t i;
	uint16_t uslen = 0;
    
	for(i=0;i<len;i++)
	{
		if (*pStr != '\0')
		{
			*pDst++ = *pStr;
			uslen++;
		}
		++pStr;
	}
	return uslen;
}



/**
 * @brief ��ʮ�������豸SNת����BCD��
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
uint8_t *ConvertSntoBcd(uint8_t *ucSrcDevSn, uint8_t *ucDestDevSn, uint32_t length)
{
	uint64_t ulDevSn;
	
    uint8_t *pBuf = (uint8_t *)&ulDevSn;
    
    pBuf[0] = ucSrcDevSn[7];
    pBuf[1] = ucSrcDevSn[6];
    pBuf[2] = ucSrcDevSn[5];
    pBuf[3] = ucSrcDevSn[4];
	pBuf[4] = ucSrcDevSn[3];
    pBuf[5] = ucSrcDevSn[2];
    pBuf[6] = ucSrcDevSn[1];
    pBuf[7] = ucSrcDevSn[0];
    
    LIB_U64DectoBCD(ulDevSn, ucDestDevSn, 8);
	
	return  ucDestDevSn;
}

/**
 * @brief ���鷴ת
 * @param[in]   
 * @param[out] 
 * @return
 * @note
 */
void str_reverse(uint8_t *pdata, uint16_t usLen)
{
    /* ��Ԫ���±� */
    uint16_t i = 0;
    /* βԪ���±� */
    uint16_t j = usLen - 1; 
    
    uint8_t uctmp;

    while(i < j)
    {
        /* ����a[i]��a[j] */
        uctmp = pdata[i];
        pdata[i] = pdata[j];
        pdata[j] = uctmp;
        /* �������� */
        i++; 
        /* �������� */
        j--; 
    }
}
