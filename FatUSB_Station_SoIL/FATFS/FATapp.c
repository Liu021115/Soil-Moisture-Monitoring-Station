#include "ff.h"
#include "string.h"
#include "stdio.h"
#include "FATapp.h"


#define FF_MAX_SS 4096

FATFS FS;
FIL FP;
BYTE work[FF_MAX_SS];
UINT BW;
char rDATA[FF_MAX_SS];
Filename Fatname;

/*****************************************************
	??????
*****************************************************/
uint8_t Fat_FreeGet (void){
	FRESULT res;
	FATFS *pfs;
  DWORD fre_clust, fre_sect;
	res = f_getfree("0:", &fre_clust, &pfs);
	if(res != FR_OK)
		return 1;
	fre_sect = fre_clust * pfs->csize;
	if(fre_sect > 0)
		return 0;
	return 1;
}

/*****************************************************
	??????
*****************************************************/
void fatfs_init (void){
	FRESULT res;
	res =f_mount(&FS,"0:",1);					
		printf("fmount %d\r\n",res);
	if(res == 13){	
		res =f_mkfs("0:",NULL,work,sizeof(work));
		printf("fmkfs %d\r\n",res);	
		res = f_mount(NULL,"0:",1);
		res =f_mount(&FS,"0:",1);
		printf("fmount again %d\r\n",res);
	}
}

/*****************************************************
	??????
*****************************************************/
void Fat_FilePath(char *filename,char *dirname,DateTime *dt){
	sprintf(dirname, "0:/%02d_%02d", dt->year, dt->month);
	sprintf(filename, "%s/%02d_%02d.txt",dirname,dt->month, dt->day);
}

/*****************************************************
??????
*****************************************************/
void	File_write (char *filename,char *dirname,DateTime *dt){
	FRESULT res;
	DIR dir;
	
	res = f_opendir(&dir,dirname);
	if (res != FR_OK) {
		if(Fat_FreeGet() == 1)
			return;
    res = f_mkdir(dirname);
    if (res != FR_OK) 
        return;
	}
	f_closedir(&dir);
	if(Fat_FreeGet() == 1)
			return;
	res = f_open(&FP,filename,FA_OPEN_ALWAYS|FA_WRITE|FA_READ);
	if(res == 0){	
			//生成存储数据帧
		char Timebuff[8]={0};
		char FatFSbuff[128] = {0};																				// 创建临时缓冲区
		sprintf(Timebuff,"%02d_%02d", dt->day, dt->hour);
		sprintf(FatFSbuff,"Time:%s;TEMP1:%.1f;HUMI1:%.1f;EC1:%.1f;TEMP2:%.1f;HUMI2:%.1f;EC2:%.1f;TEMP3:%.1f;HUMI3:%.1f;EC3:%.1f;\r\n",
					Timebuff,SoilData.tempe1.value,SoilData.humi1.value,SoilData.EC1.value,
					SoilData.tempe2.value,SoilData.humi2.value,SoilData.EC2.value,
					SoilData.tempe3.value,SoilData.humi3.value,SoilData.EC3.value);
		f_lseek(&FP, f_size(&FP));
		res = f_write(&FP,FatFSbuff,strlen(FatFSbuff),&BW);
		}
		f_close(&FP);	
}
/*
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
*/

