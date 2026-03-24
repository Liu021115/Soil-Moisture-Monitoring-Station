#ifndef __SOIL_H
#define __SOIL_H

#include "stm32f10x.h"
#include "RS485.h"
#include "string.h"
#include <stdio.h>

void SoilCheck_Cmd1(void);
void SoilCheck_Cmd2(void);
void SoilCheck_Cmd3(void);
void Soil_Printf(void);

#endif
