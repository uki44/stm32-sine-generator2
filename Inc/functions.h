#ifndef __functions_H
#define __functions_H
#endif


#include "stdint.h"
#include "math.h"
#include "main.h"
#include "stm32l4xx_hal.h"


#define PI 3.141592653

void calcsin(uint32_t *sin_arr, uint8_t V);
int ARR_Cal(float freq);
void setARR(float *values, uint8_t n);
float assembleFloat(uint8_t* valArr, uint8_t index);
int assembleInt(uint8_t *valArr,uint8_t index);
void debug_print_array(uint8_t* arr,int size);
void processData(float* freq_arr,float* voltage_arr,int* time_arr,uint8_t* buff_arr1,uint8_t* buff_arr2);
int prescCalc(int* time,int index);
uint32_t TIM_GetCounter(TIM_TypeDef* TIMx);
void TIM_resetCounder(TIM_TypeDef* TIMx);
void TIM_setPrescaler(TIM_TypeDef* TIMx,int val);
void setDigiPot(I2C_HandleTypeDef* I2C,float voltage, uint8_t digiPotAddr);
void setDigiPot2(I2C_HandleTypeDef* I2C,float voltage, uint8_t digiPotAddr);
void debugI2Cscan(I2C_HandleTypeDef *hi2cx,UART_HandleTypeDef *huartx);
void savePreset(float *floatArr,float *voltageArr,int *timeArr,I2C_HandleTypeDef *hi2cx,uint16_t eeprom_addr);
void writeDataInfoToScreen(char msgString[4][50],float*  float_arr,float* voltage_arr, int* time_arr,int set,uint8_t cursor_pos_x,uint8_t cursor_pos_y);
void displayInitData();
void initDigiPot(I2C_HandleTypeDef* i2cx,uint8_t device_addr);
void EEPROM_Write(I2C_HandleTypeDef* i2cx,uint8_t eeprom_addr,uint16_t page, uint16_t offset, uint8_t *data, uint16_t size);
void EEPROM_Read(I2C_HandleTypeDef* i2cx,uint8_t eeprom_addr,uint16_t page, uint16_t offset, uint8_t *data, uint16_t size);
void EEPROM_PageErase (I2C_HandleTypeDef* i2cx,uint8_t eeprom_addr,uint16_t page);
uint16_t bytestowrite (uint16_t size, uint16_t offset);
void savePreset(float *floatArr,float *voltageArr,int *timeArr,I2C_HandleTypeDef *hi2cx,uint16_t eeprom_addr);
void readPreset(float *floatArr,float *voltageArr,int *timeArr,I2C_HandleTypeDef *hi2cx,uint16_t eeprom_addr);
void writeToFlash(uint8_t* arr);
void readFromFlash(uint8_t* arr);
uint32_t Flash_Write_Data (uint32_t StartPageAddress, uint64_t *Data, uint16_t numberofwords);
void Flash_Read_Data (uint32_t StartPageAddress, uint32_t *RxBuf, uint16_t numberofwords);