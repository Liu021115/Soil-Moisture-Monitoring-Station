// protocol_def.h
#ifndef __PROTOCOL_DEF_H
#define __PROTOCOL_DEF_H

#include <stdint.h>
#include <stdbool.h>

// 系统编码定义
#define SYSTEM_CODE_SOIL_QUALITY     25

// 命令编码定义
#define CMD_DATA_UPLOAD            2011


// 状态标志定义
#define FLAG_NORMAL                'N'
#define FLAG_ABNORMAL              'A'
#define SOIL_ERROR                 'B'


#pragma pack(1)
typedef struct {
    uint8_t year;   // 年 (00-99)
    uint8_t month;  // 月 (1-12)
    uint8_t day;    // 日 (1-31)
		uint8_t	week;
    uint8_t hour;   // 时 (0-23)
    uint8_t min;    // 分 (0-59)
    uint8_t sec;    // 秒 (0-59)
} DateTime;

typedef struct {
    float value;    // 数值
    char flag;      // 标志(N正常, B异常)
} DataItem;


typedef struct {
    DateTime dataTime;      // 数据采集时间
    uint8_t realFlag;       // 实时标志
    DataItem tempe1;   			// 温度(A87000)
    DataItem humi1;      		// 湿度(A87001)
    DataItem EC1;     			// 电导率(A87005)
    DataItem tempe2;   			// 温度(A87100)
    DataItem humi2;      		// 湿度(A87101)
    DataItem EC2;     			// 电导率(A87105)
    DataItem tempe3;   			// 温度(A87200)
    DataItem humi3;      		// 湿度(A87201)
    DataItem EC3;     			// 电导率(A87205)
} SoilmoistureData;      	//实时监测数据结构体

typedef struct {
    char Lat[16];			//纬度
    char Lon[16];			//经度
}positioning_;        //经纬度信息


#pragma pack()

extern  SoilmoistureData SoilData;
extern	positioning_ positioning;
extern  DateTime RTCTime;
// CRC16计算函数
uint16_t CalculateCRC16(const uint8_t *data, uint16_t length);

//  数据上传函数
void SoilData2Web( char *deviceId,  char *password);
uint32_t SoilDate2Flash(uint32_t PageAddress);

void Location2Web(DateTime *dt,char *deviceId, char *password);
void CID2Web(DateTime *dt,char *deviceId, char *password, char *CID);
#endif



