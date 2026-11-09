#ifndef APOLLO_BSP_H
#define APOLLO_BSP_H

#include "stm32n6xx_hal.h"
#include "apollo_error.h"
#include "i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported variables and functions */
extern I2C_HandleTypeDef *hbus_i2c1;

int32_t BSP_I2C1_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C1_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C1_WriteReg16(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C1_ReadReg16(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C1_IsReady(uint16_t DevAddr, uint32_t Trials);
int32_t BSP_GetTick(void);


#ifdef __cplusplus
}
#endif

#endif /* APOLLO_BSP_H */
