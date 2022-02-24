#include "functions.h"
#include "stdint.h"
#include "stm32l4xx_hal.h"
#include "main.h"
#include "tim.h"
#include "math.h"
#include "usbd_custom_hid_if.h"


void calcsin(uint32_t *sin_arr, uint8_t V)
{
	int n = 255;

	// uint8_t v_out = V*(4096/3.3);

	for (int i = 0; i < n; i++)
	{

		sin_arr[i] = ((sin(i * 2 * PI / n) + 1) * ((4096 - 100) / 2)) + 50;
		// sin_arr[i] = ((sin(i * 2 * PI / n) + 1) * (4096 / 2));
	}
}

int ARR_Cal(float freq)
{
	int TIM_CLK = 48 * pow(10, 6); // timer frequency
	int n = 256;				   // number of samples
	int PRESC = 48 - 1;			   // timer prescaler that divides the frequency to 1MHz ( if set to 72-1);
	float ARR_dec;				   // decimal ARR value
	int ARR_Int;
	ARR_dec = (TIM_CLK / (PRESC * freq * n));
	ARR_Int = round(ARR_dec);
	return ARR_Int - 1;
}
void setARR(uint8_t *values, uint8_t n)
{
	int ARR_Val;
	float freq;

	freq = getFreq(values,n);

	ARR_Val = ARR_Cal(freq);

	if (ARR_Val >= 0)
	{
		// TIM2->ARR = ARR_Val; // zapiše vrednost v register
		__HAL_TIM_SET_AUTORELOAD(&htim2, ARR_Val);
		//__HAL_TIM_SET_COUNTER(&htim2, ARR_Val);
	}
	if (ARR_Val < 0)
	{

		Error_Handler();
	}
}
float getVoltage(uint8_t *arr, uint8_t n)
{

	return 0; // added to remove compiler warning, delete later
}

float getFreq(uint8_t *arr, uint8_t n)
{
	float freqn = 0; 
	buffer1[1];

	return 10; // added to remove compiler warning, delete later
}
int getDuration(uint8_t *arr, uint8_t n)
{

	return 0; // added to remove compiler warning, delete later
}
