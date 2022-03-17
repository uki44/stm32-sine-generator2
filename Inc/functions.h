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
float getVoltage(uint8_t* arr, uint8_t n);
float getFreq(uint8_t* arr, uint8_t n);
int getDuration(uint8_t* arr, uint8_t n);
float assembleFloat(uint8_t* valArr, uint8_t index);