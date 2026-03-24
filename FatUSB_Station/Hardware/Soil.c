#include "Soil.h"
#include "protocol_def.h"
/*****************************************************
下面就是需要修改的地方，修改设备地址、功能码、CRC  
*****************************************************/
#define SoilID_1 0x60
#define SoilID_2 0x61
#define SoilID_3 0x62
#define SoilDataRead_1 0x03
#define SoilDataRead_2 0x03
#define SoilDataRead_3 0x03
#define SoilDataCRC_1H 0x0D
#define SoilDataCRC_1L 0xBA
#define SoilDataCRC_2H 0x0C
#define SoilDataCRC_2L 0x6B
#define SoilDataCRC_3H 0x0C
#define SoilDataCRC_3L 0x58

#define WEATHER_WAIT_MAX	500
#define WEATHER_WAIT_STEP	20 

uint8_t Soil_Data1[11];      //墒情数据接收缓冲区
uint8_t Soil_Data2[11];      //墒情数据接收缓冲区
uint8_t Soil_Data3[11];      //墒情数据接收缓冲区

uint8_t Soil_Check1[8]={SoilID_1,SoilDataRead_1,0x00,0x00,0x00,0x03,SoilDataCRC_1H,SoilDataCRC_1L};  //查询1层土壤指令
uint8_t Soil_Check2[8]={SoilID_2,SoilDataRead_2,0x00,0x00,0x00,0x03,SoilDataCRC_2H,SoilDataCRC_2L};  //查询2层土壤指令
uint8_t Soil_Check3[8]={SoilID_3,SoilDataRead_3,0x00,0x00,0x00,0x03,SoilDataCRC_3H,SoilDataCRC_3L};  //查询3层土壤指令
uint16_t soilcrc;                  //墒情CRC校验
char Soilflag1;
char Soilflag2;
char Soilflag3;

/*****************************************************
打印各层土壤数据
*****************************************************/
void Soil_Printf(void)
{
	printf("Soil_10cm:");
	for(uint8_t i=0;i<11;i++)
	printf("0x%02X ",Soil_Data1[i]);
	printf("\r\n");
	
	printf("Soil_20cm:");
	for(uint8_t i=0;i<11;i++)
	printf("0x%02X ",Soil_Data2[i]);
	printf("\r\n");	
	
	printf("Soil_30cm:");
	for(uint8_t i=0;i<11;i++)
	printf("0x%02X ",Soil_Data3[i]);
	printf("\r\n");
}



/*****************************************************
获取1层土壤数据命令
*****************************************************/
void SoilCheck_Cmd1(void)
{
	uint16_t wait_time = 0;   //延时等待时间
	//将RS485接收缓冲区设为11位  
	rx_idex = 0;                   
	rx_max = 11;
	//发送查询指令	
	RS485_Send(Soil_Check1,8);
	//循环等待接收完成
	while(rx_idex < rx_max-1 &&	wait_time < WEATHER_WAIT_MAX){
		Delay_ms(WEATHER_WAIT_STEP);
		wait_time += WEATHER_WAIT_STEP;
	}
	//判断接收的CRC校验位	
	soilcrc = ModbusCRC16(uart3_data,rx_max-2);	
	//数据copy
	if(uart3_data[0]==SoilID_1 &&  uart3_data[1] ==SoilDataRead_1 && soilcrc == ((uart3_data[10] << 8 | uart3_data[9])))
	{
		memset(Soil_Data1,0,rx_max);
		Delay_us(10);
		Soilflag1 = FLAG_NORMAL;
		memcpy(Soil_Data1,uart3_data,rx_max);
		Delay_us(10);
		SoilData.tempe1.value =(float)(int16_t)(((uint16_t)Soil_Data1[3] << 8) | Soil_Data1[4])/10; //摄氏度
		SoilData.humi1.value = (float)((Soil_Data1[5]<<8)+Soil_Data1[6])/10;			 //土壤湿度 %
		SoilData.EC1.value = (float)((Soil_Data1[7]<<8)+Soil_Data1[8]);     	   //电导率EC
		memset(uart3_data,0,rx_max+1);
	}
	else
		Soilflag1 = SOIL_ERROR;
	
	SoilData.tempe1.flag = Soilflag1;
  SoilData.humi1.flag = Soilflag1;									
  SoilData.EC1.flag = Soilflag1;
}

/*****************************************************
获取2层土壤数据命令
*****************************************************/
void SoilCheck_Cmd2(void)
{
	uint16_t wait_time = 0;   //延时等待时间
	//将RS485接收缓冲区设为11位  
	rx_idex = 0;                   
	rx_max = 11;
	//发送查询指令	
	RS485_Send(Soil_Check2,8);
	while(rx_idex < rx_max-1 &&	wait_time < WEATHER_WAIT_MAX){
		Delay_ms(WEATHER_WAIT_STEP);
		wait_time += WEATHER_WAIT_STEP;
	}
	//判断接收的CRC校验位	
	soilcrc = ModbusCRC16(uart3_data,rx_max-2);	
	//数据copy
	if(uart3_data[0]==SoilID_2 &&  uart3_data[1] ==SoilDataRead_2 && soilcrc == ((uart3_data[10] << 8 | uart3_data[9])))
	{
		memset(Soil_Data2,0,rx_max);
		Delay_us(10);
		Soilflag2 = FLAG_NORMAL;
		memcpy(Soil_Data2,uart3_data,11);		
		Delay_us(10);
		SoilData.tempe2.value =(float)(int16_t)(((uint16_t)Soil_Data2[3] << 8) | Soil_Data2[4])/10; //摄氏度
		SoilData.humi2.value = (float)((Soil_Data2[5]<<8)+Soil_Data2[6])/10;			 //土壤湿度 %
		SoilData.EC2.value = (float)((Soil_Data2[7]<<8)+Soil_Data2[8]);     	   //电导率EC
		memset(uart3_data,0,rx_max+1);
	}
	else
		Soilflag2 = SOIL_ERROR;
	
	SoilData.tempe2.flag = Soilflag2;
  SoilData.humi2.flag = Soilflag2;									
  SoilData.EC2.flag = Soilflag2;
}


/*****************************************************
获取3层土壤数据命令
*****************************************************/
void SoilCheck_Cmd3(void)
{
	uint16_t wait_time = 0;   //延时等待时间
	//将RS485接收缓冲区设为11位  
	rx_idex = 0;                   
	rx_max = 11;
	//发送查询指令	
	RS485_Send(Soil_Check3,8);
	while(rx_idex < rx_max-1 &&	wait_time < WEATHER_WAIT_MAX){
		Delay_ms(WEATHER_WAIT_STEP);
		wait_time += WEATHER_WAIT_STEP;
	}
	//判断接收的CRC校验位	
	soilcrc = ModbusCRC16(uart3_data,rx_max-2);	
	//数据copy
	if(uart3_data[0]==SoilID_3 &&  uart3_data[1] ==SoilDataRead_3  && soilcrc == ((uart3_data[10] << 8 | uart3_data[9]))) 
	{
		memset(Soil_Data3,0,rx_max);
		Delay_us(10);
		Soilflag3 = FLAG_NORMAL;
		memcpy(Soil_Data3,uart3_data,11);
		Delay_us(10);
		SoilData.tempe3.value =(float)(int16_t)(((uint16_t)Soil_Data3[3] << 8) | Soil_Data3[4])/10; //摄氏度
		SoilData.humi3.value = (float)((Soil_Data3[5]<<8)+Soil_Data3[6])/10;			 //土壤湿度 %
		SoilData.EC3.value = (float)((Soil_Data3[7]<<8)+Soil_Data3[8]);     	   //电导率EC
		memset(uart3_data,0,rx_max+1);
	}
	else
		Soilflag3 = SOIL_ERROR;
	
	SoilData.tempe3.flag = Soilflag3;
  SoilData.humi3.flag = Soilflag3;									
  SoilData.EC3.flag = Soilflag3;
}
