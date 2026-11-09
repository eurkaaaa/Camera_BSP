#include "apollo.h"
#include <string.h>

/* I2C handle */
I2C_HandleTypeDef *hbus_i2c1 = &hi2c1;

int32_t BSP_I2C1_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
    if (HAL_I2C_Mem_Write(hbus_i2c1, DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, 1000) == HAL_OK)
        return BSP_ERROR_NONE;
    return BSP_ERROR_PERIPH_FAILURE;
}

int32_t BSP_I2C1_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
    if (HAL_I2C_Mem_Read(hbus_i2c1, DevAddr, Reg, I2C_MEMADD_SIZE_8BIT, pData, Length, 1000) == HAL_OK)
        return BSP_ERROR_NONE;
    return BSP_ERROR_PERIPH_FAILURE;
}

int32_t BSP_I2C1_WriteReg16(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
    if (HAL_I2C_Mem_Write(hbus_i2c1, DevAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, 1000) == HAL_OK)
        return BSP_ERROR_NONE;
    return BSP_ERROR_PERIPH_FAILURE;
}

int32_t BSP_I2C1_ReadReg16(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
    if (HAL_I2C_Mem_Read(hbus_i2c1, DevAddr, Reg, I2C_MEMADD_SIZE_16BIT, pData, Length, 1000) == HAL_OK)
        return BSP_ERROR_NONE;
    return BSP_ERROR_PERIPH_FAILURE;
}

int32_t BSP_I2C1_IsReady(uint16_t DevAddr, uint32_t Trials)
{
    if (HAL_I2C_IsDeviceReady(hbus_i2c1, DevAddr, Trials, 1000) == HAL_OK) return BSP_ERROR_NONE;
    return BSP_ERROR_BUSY;
}

int32_t BSP_GetTick(void)
{
  return (int32_t)HAL_GetTick();
}

