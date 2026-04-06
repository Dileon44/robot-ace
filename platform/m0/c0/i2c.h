#ifndef __I2C_H
#define __I2C_H

#include "main.h"
#include "platform.h"
#include "platform_inc_m0.h"

bool I2C_Sensor_Init(void);
bool I2C_Sensor_DeInit(void);
RET_STATE_t I2C_Sensor_Read(u8 devAddr, u8 wordAddr, u8* pData, u16 len);
RET_STATE_t I2C_Sensor_Write(u8 devAddr, u8 wordAddr, const u8* pData, u16 len);

#endif /* __I2C_H */
