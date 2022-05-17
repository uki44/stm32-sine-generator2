#include "functions.h"
#include "stdint.h"
#include "stm32l4xx_hal.h"
#include "main.h"
#include "tim.h"
#include "math.h"
#include "usbd_custom_hid_if.h"
#include "string.h"

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
	int TIM_CLK = 32 * pow(10, 6); // timer frequency
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

	freq = getFreq(values, n);

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
	freqn = buffer1[1];
	UNUSED(freqn); // added to supress compiler warning, delete later
	return 10;	   // added to remove compiler warning, delete later
}
int getDuration(uint8_t *arr, uint8_t n)
{

	return 0; // added to remove compiler warning, delete later
}

float assembleFloat(uint8_t *valArr, uint8_t index)
{

	uint8_t tempArr[4];
	uint8_t maxInd = index + 4;
	float retVal;

	for (int i = 0; index < maxInd; index++)
	{

		tempArr[i] = valArr[index];
		i++;
	}

	memcpy(&retVal, tempArr, sizeof(retVal));

	return retVal;
}

void debug_print_array(uint8_t *arr, int size)
{

	for (int i = 0; i < size; i++)
	{

		debug_printf("%d: %u \r\n", i, arr[i]);
	}

	return;
}


void processData(float *freq_arr, float *voltage_arr, int *time_arr, uint8_t *buff_arr1, uint8_t *buff_arr2)
{

	int arr_pos = 2, arr_pos2 = 2;
	

	for (int i = 0, snum = 0; i < 5; i++)
	{
		freq_arr[snum] = assembleFloat(buff_arr1, arr_pos);
		arr_pos += 4;
		voltage_arr[snum] = assembleFloat(buff_arr1, arr_pos);
		arr_pos += 4;
		time_arr[snum] = buff_arr1[arr_pos];
		arr_pos += 1;
		snum++;
	}

	for (int i = 0, snum = 5; i < 5; i++)
	{
		freq_arr[snum] = assembleFloat(buff_arr2, arr_pos2);
		arr_pos2 += 4;
		voltage_arr[snum] = assembleFloat(buff_arr2, arr_pos2);
		arr_pos2 += 4;
		time_arr[snum] = buff_arr1[arr_pos2];
		arr_pos2 += 1;
		snum++;
	}

	return;
}

int prescCalc(int* time,int index){

	int prescVal;
	int TIM_CLK = 32 * pow(10, 6);
	unsigned long int ARR = pow(2,32) - 1;


	prescVal= (TIM_CLK*time[index]*60)/ARR;

	return prescVal;

}
uint32_t TIM_GetCounter(TIM_TypeDef* TIMx){

	/* preveri če so parametri pravilni*/
	assert_param(IS_TIM_ALL_PERIPH(TIMx)); 

	/*vrne trenutno vrednost števca*/
	return TIMx->CNT;

}
void TIM_resetCounder(TIM_TypeDef* TIMx){

	/* preveri če so parametri pravilni*/
	assert_param(IS_TIM_ALL_PERIPH(TIMx));

	/*zapiše vrednost 0 v števec*/
	TIMx->CNT = 0;


}
void setDigiPot(float* voltageArr, uint8_t digiPotAddr){



}