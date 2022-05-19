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
	int PRESC = 48 - 1;			   // timer prescaler that divides the frequency to 1MHz ( if set to 32 - 1 );
	float ARR_dec;				   // decimal ARR value
	int ARR_Int;
	ARR_dec = (TIM_CLK / (PRESC * freq * n));
	ARR_Int = round(ARR_dec);
	return ARR_Int - 1;
}
void setARR(float *values, uint8_t n)
{
	int ARR_Val;
	float freq;

	freq = values[n];

	ARR_Val = ARR_Cal(freq);

	if (ARR_Val > 0)
	{
		// TIM2->ARR = ARR_Val; // zapiše vrednost v register
		__HAL_TIM_SET_AUTORELOAD(&htim2, ARR_Val);
		//__HAL_TIM_SET_COUNTER(&htim2, ARR_Val);
	}
	if (ARR_Val <= 0)
	{

		Error_Handler();
	}
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
	

	for (int i = 0, snum = 0; i < 5; i++)   // converts the first half of the data set from int to float 
	{
		freq_arr[snum] = assembleFloat(buff_arr1, arr_pos);
		arr_pos += 4;
		voltage_arr[snum] = assembleFloat(buff_arr1, arr_pos);
		arr_pos += 4;
		time_arr[snum] = buff_arr1[arr_pos];
		arr_pos += 1;
		snum++;
	}

	for (int i = 0, snum = 5; i < 5; i++)  // converts the second half of the data set from int to float 
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
void setDigiPot(float* voltageArr, uint8_t digiPotAddr){  //writes the value to the I2C digital potentiometer 



}

void debugI2Cscan(I2C_HandleTypeDef *hi2cx,UART_HandleTypeDef *huartx){

	uint8_t Buffer[25] = {0};
	uint8_t Space[] = " - ";
	uint8_t StartMSG[] = "Starting I2C Scanning: \r\n";
	uint8_t EndMSG[] = "Done! \r\n\r\n";
	uint8_t ret;

	HAL_UART_Transmit(huartx, StartMSG, sizeof(StartMSG), 10000);

	for(uint8_t i=1; i<128; i++)
    {
        ret = HAL_I2C_IsDeviceReady(hi2cx, (uint16_t)(i<<1), 3, 5);
        if (ret != HAL_OK) /* No ACK Received At That Address */
        {
            HAL_UART_Transmit(huartx, Space, sizeof(Space), 10000);
        }
        else if(ret == HAL_OK)
        {
            sprintf(Buffer, "0x%X", i);
            HAL_UART_Transmit(huartx, Buffer, sizeof(Buffer), 10000);
        }
    }
	HAL_UART_Transmit(huartx, EndMSG, sizeof(EndMSG), 10000);

	return;
}
void savePreset(float *floatArr,float *voltageArr,int *timeArr,I2C_HandleTypeDef *hi2cx,uint16_t EEPROM_ADDR){
	
	uint8_t presetArr[256];
	uint8_t *p_presetArr = presetArr;

	memcpy(p_presetArr, floatArr,sizeof(floatArr));
	p_presetArr += sizeof(floatArr);
	memcpy(p_presetArr, voltageArr,sizeof(voltageArr));
	p_presetArr += sizeof(voltageArr);
	memcpy(p_presetArr, timeArr,sizeof(timeArr));
	p_presetArr += sizeof(timeArr);

	writeToEEPROM(hi2cx,presetArr,EEPROM_ADDR);
	
	return;	

}

void writeToEEPROM(I2C_HandleTypeDef *hi2cx,uint8_t *dataArr,uint16_t EEPROM_ADDR){

	HAL_I2C_Master_Transmit(hi2cx, EEPROM_ADDR,dataArr,sizeof(dataArr),100);

	return;

}
