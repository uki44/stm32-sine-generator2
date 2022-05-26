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
void debug_print_array(uint8_t* arr,int size);
void processData(float* freq_arr,float* voltage_arr,int* time_arr,uint8_t* buff_arr1,uint8_t* buff_arr2);
int prescCalc(int* time,int index);
uint32_t TIM_GetCounter(TIM_TypeDef* TIMx);
void TIM_resetCounder(TIM_TypeDef* TIMx);
void setDigiPot(float* voltageArr, uint8_t digiPotAddr);
void debugI2Cscan(I2C_HandleTypeDef *hi2cx,UART_HandleTypeDef *huartx);
void savePreset(float *floatArr,float *voltageArr,int *timeArr,I2C_HandleTypeDef *hi2cx,uint16_t EEPROM_ADDR);
void writeToEEPROM(I2C_HandleTypeDef *hi2cx,uint8_t *dataArr,uint16_t EEPROM_ADDR);
void EEPROMfetchPreset(I2C_HandleTypeDef *hi2cx,uint8_t *dataArr,uint16_t EEPROM_ADDR,float* float_Arr, float* voltage_Arr,int* time_Arr);