#include "protocol_def.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "4G.h"
#include "W25Q128.h"
#include "24LC64F.h"

char response[1024];        		 //发送数据包缓冲区
char packetBody[1024];					 //协议生成缓冲
char cpBuffer[512];         		 //CP段缓冲区   不放在函数里防止栈溢出
extern	float Battery_V;
extern DateTime RTCTime;
SoilmoistureData SoilData;	 //气象实时数据结构体
/**************************************************************************************** 
函 数: CRC16_Checkout 
描 述: CRC16 循环冗余校验算法。 
参 数 一: *puchMsg：需要校验的字符串指针 
参 数 二: usDataLen：要校验的字符串长度 
返 回 值: 返回 CRC16 校验码 
****************************************************************************************/ 
//测试一下交换字节序   return ((crc_reg>>8) | (crc_reg<<8));
uint16_t CalculateCRC16 ( const uint8_t *puchMsg, uint16_t usDataLen ) 
{ 
	unsigned int i,j,crc_reg,check; 
 
	crc_reg = 0xFFFF; 
	for(i=0;i<usDataLen;i++) 
	{ 
		crc_reg = (crc_reg>>8) ^ puchMsg[i]; 
		for(j=0;j<8;j++) 
		{ 
			check = crc_reg & 0x0001; 
			crc_reg >>= 1; 
			if(check==0x0001)
			{ 
				crc_reg ^= 0xA001; 
			} 
		} 
	} 
	return crc_reg; 
} 


/**************************************************************************************** 
函 数: 日期生成函数
描 述: 将RTC结构体时间转换成上传用的字符串 
参 数 一: *puchMsg：需要校验的字符串指针 
参 数 二: usDataLen：要校验的字符串长度 
返 回 值: 返回 CRC16 校验码 
****************************************************************************************/ 
static void generateDateTimeStr(const DateTime *dt, char *buf)
{
    sprintf(buf, "%04d%02d%02d%02d%02d%02d", 
            dt->year+2000, dt->month, dt->day, 
            dt->hour, dt->min, dt->sec);
}


/**************************************************************************************** 
函 数: 实时数据协议包生成 
描 述: 生成实时数据协议包并打印到response缓冲区
参 数: 气象数据 设备MN 密码 协议包缓冲区等指针 
返 回 值: 返回0为正常   -1错误码
****************************************************************************************/ 
int GenerateSoilDataPacket(SoilmoistureData *data, const char *deviceId, 
                             const char *password, char *output, uint16_t maxLen) {	
    char timeStr[15];   												    // YYYYMMDDhhmmss (14字符) + null终止
    char qnStr[18];   														  // QN格式: YYYYMMDDhhmmssSSS (17字符) + null终止   
    // 1. 生成QN（时间戳）
    generateDateTimeStr(&data->dataTime, timeStr);	// 生成时间字符串到缓冲区
    sprintf(qnStr, "%.14s000", timeStr); 						// 取前14位时间+3位毫秒(根据实际需求毫秒可更改)															 												 																											 															 
    // 2. 生成CP段内容
		char CP_String[512] = {0};         							//CP段数据格式换缓冲区   预留足够的长度防止后续加传感器后不够用 
									 
    sprintf(CP_String, "DataTime=%s;"
            "A87000-Rtd=%.1f,A87000-Flag=%c;"				// 将传感器数据串打印到CP_String
            "A87001-Rtd=%.1f,A87001-Flag=%c;"
            "A87005-Rtd=%.1f,A87005-Flag=%c;"
            "A87100-Rtd=%.1f,A87100-Flag=%c;"
            "A87101-Rtd=%.1f,A87101-Flag=%c;"
            "A87105-Rtd=%.1f,A87105-Flag=%c;"
            "A87200-Rtd=%.1f,A87200-Flag=%c;"
						"A87201-Rtd=%.1f,A87201-Flag=%c;"
						"A87205-Rtd=%.1f,A87205-Flag=%c;",
						timeStr,
            data->tempe1.value, data->tempe1.flag,
            data->humi1.value, data->humi1.flag,
            data->EC1.value, data->EC1.flag,
            data->tempe2.value, data->tempe2.flag,
            data->humi2.value, data->humi2.flag,
            data->EC2.value, data->EC2.flag,
            data->tempe3.value, data->tempe3.flag,
            data->humi3.value, data->humi3.flag,
            data->EC3.value, data->EC3.flag);							

    // 3. 生成协议包主体
    sprintf(packetBody, "QN=%s;ST=%d;CN=%d;PW=%s;MN=%s;Flag=%d;PNUM=1;PNO=1;CP=&&%s&&",
            qnStr,                  								// 使用生成的QN字符串
            SYSTEM_CODE_SOIL_QUALITY, 							// ST码   土壤墒情传感上传数据为25
            CMD_DATA_UPLOAD,												// CN码   土壤墒情传感上传数据为2011
            password, 															// 密码
            deviceId, 															// 设备MN码
            5,                      								// Flag=5表示不分包
            CP_String);   										
    // 4. 计算CRC校验码
    uint16_t crc = CalculateCRC16((uint8_t *)packetBody, strlen(packetBody));
    // 5. 计算字节数（CP段ASCII码字符数）
    uint16_t byteCount = strlen(CP_String);
    // 6. 生成完整包
    uint16_t totalLen = 2 + 4 + strlen(packetBody) + 4; // ## + 4位字节数 + 包主体 + 4位CRC
    
    if (totalLen > maxLen) {														// 判断缓冲区是否溢出
			printf("缓冲区不足\r\n");
        return -1; 																			// 缓冲区不足返回
    }
		memset(response,0,sizeof(response));
    sprintf(response, "##%04d%s%04X\r\n", strlen(packetBody), packetBody, crc);  //将最终协议包打印到缓冲区
		
		
    return 0;
}


/**************************************************************************************** 
函 数: 协议包上传函数
参 数 一: *deviceId：设备MN码数组指针
参 数 一: *password：设备密码数组指针
返 回 值: 无
****************************************************************************************/ 
void SoilData2Web( char *deviceId, char *password)
{

		SoilData.dataTime = RTCTime;                        //将RTC时间同步到数据结构体
		GenerateSoilDataPacket(&SoilData, deviceId, password,response, sizeof(response));
																													 //生成协议包
		char LenBuff[10];																			 //字符串长度临时缓冲区
		sprintf(LenBuff, "%d",strlen(response));  						 //将长度转换为字符串 
		CSTX_4G_Senddata((uint8_t*)LenBuff, response);				 //4G模块上传协议包	
}


/**************************************************************************************** 
函 数: 上传定位信息
****************************************************************************************/ 
void Location2Web(DateTime *dt,char *deviceId, char *password)
{	
	  char timeStr[15];   												    // YYYYMMDDhhmmss (14字符) + null终止
    char qnStr[18];   														  // QN格式: YYYYMMDDhhmmssSSS (17字符) + null终止   
    // 1. 生成QN（时间戳）
    generateDateTimeStr(dt, timeStr);	// 生成时间字符串到缓冲区
    sprintf(qnStr, "%.14s000", timeStr); 						// 取前14位时间+3位毫秒(根据实际需求毫秒可更改)															 												 																											 															 
    // 2. 生成CP段内容
		char CP_String[64] = {0};
		char Data_String[256] = {0};    

    snprintf(CP_String, sizeof(CP_String),
						"DataTime=%s;Lng=%s,Lat=%s;SB1RS=1",				// 将传感器数据串打印到CP_String,
						timeStr,positioning.Lon,positioning.Lat);							
    // 3. 生成协议包主体
    sprintf(Data_String, "QN=%s;ST=73;CN=3041;PW=%s;MN=%s;Flag=%d;CP=&&%s&&",
            qnStr,                  								// 使用生成的QN字符串
            password, 															// 密码
            deviceId, 															// 设备MN码
            8,                      								// Flag=5表示不分包
            CP_String);   										
    // 4. 计算CRC校验码
		uint16_t packetBodyLen = strlen(Data_String);
    uint16_t crc = CalculateCRC16((uint8_t *)Data_String, packetBodyLen);
    // 5. 计算字节数（CP段ASCII码字符数）
    uint16_t byteCount = strlen(CP_String);
		// 6. 生成完整协议包
		memset(response,0,sizeof(response));
    sprintf(response, "##%04d%s%04X\r\n", packetBodyLen, Data_String, crc);  //将最终协议包打印到缓冲区
		
		char LenBuff[8];																			 //字符串长度临时缓冲区
		sprintf(LenBuff, "%d",strlen(response));  						 //将长度转换为字符串 

		CSTX_4G_Senddata((uint8_t*)LenBuff, response);				 //4G模块上传协议包	
}

/**************************************************************************************** 
函 数: 上传CID
****************************************************************************************/ 
void CID2Web(DateTime *dt,char *deviceId, char *password, char *CID)
{
	  char timeStr[15];   												    // YYYYMMDDhhmmss (14字符) + null终止
    char qnStr[18];   														  // QN格式: YYYYMMDDhhmmssSSS (17字符) + null终止   
    // 1. 生成QN（时间戳）
    generateDateTimeStr(dt, timeStr);	// 生成时间字符串到缓冲区
    sprintf(qnStr, "%.14s000", timeStr); 						// 取前14位时间+3位毫秒(根据实际需求毫秒可更改)															 												 																											 															 
    // 2. 生成CP段内容
		char CP_String[64] = {0};
		char Data_String[256] = {0};      
    snprintf(CP_String, sizeof(CP_String),
						"DataTime=%s;ICCID=%s",				// 将传感器数据串打印到CP_String,
						timeStr,CID);							
    // 3. 生成协议包主体
    sprintf(Data_String, "QN=%s;ST=74;CN=2083;PW=%s;MN=%s;Flag=8;CP=&&%s&&",
            qnStr,                  								// 使用生成的QN字符串
            password, 															// 密码
            deviceId, 															// 设备MN码
            CP_String);   										
    // 4. 计算CRC校验码
		uint16_t packetBodyLen = strlen(Data_String);
    uint16_t crc = CalculateCRC16((uint8_t *)Data_String, packetBodyLen);
    // 5. 计算字节数（CP段ASCII码字符数）
    uint16_t byteCount = strlen(CP_String);
		// 6. 生成完整协议包
				memset(response,0,sizeof(response));
    sprintf(response, "##%04d%s%04X\r\n", packetBodyLen, Data_String, crc);  //将最终协议包打印到缓冲区
		
		char LenBuff[8];																			 //字符串长度临时缓冲区
		sprintf(LenBuff, "%d",strlen(response));  						 //将长度转换为字符串 
		CSTX_check();
		CSTX_4G_Senddata((uint8_t*)LenBuff, response);				 //4G模块上传协议包		
	
}


