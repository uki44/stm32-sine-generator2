#include "functions.h"
#include "stdint.h"
#include "stm32l4xx_hal.h"
#include "main.h"


int n = 255;
void calcsin(uint32_t *sin_arr, uint8_t V)
{

	//uint8_t v_out = V*(4096/3.3);

	for (int i = 0; i < n; i++)
	{

		sin_arr[i] = ((sin(i * 2 * PI / n) + 1) * ((4096-100) / 2))+50;
		//sin_arr[i] = ((sin(i * 2 * PI / n) + 1) * (4096 / 2));
	}
}

int ARR_Cal(float freq)
{
	int TIM_CLK = 72 * pow(10, 6); // timer frequency
	int n = 256;				   // number of samples
	int PRESC = 72-1;			   // timer prescaler that divides the frequency to 1MHz ( if set to 72-1);
	float ARR_dec;				   // decimal ARR value
	int ARR_Int;
	ARR_dec = (TIM_CLK / (PRESC * freq * n));
	ARR_Int = round(ARR_dec);
	return ARR_Int -1;
}