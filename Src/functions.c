#include "functions.h"
#include "stdint.h"
#include "stm32l4xx_hal.h"
#include "main.h"
#include "tim.h"
#include "math.h"
#include "usbd_custom_hid_if.h"
#include "string.h"
#include "ssd1306.h"

void calcsin(uint32_t *sin_arr, uint8_t V)  //init function that creates the array with precalculated values for the sine-wave
{
	int n = 255;

	// uint8_t v_out = V*(4096/3.3);

	for (int i = 0; i < n; i++)
	{

		sin_arr[i] = ((sin(i * 2 * PI / n) + 1) * ((4096 - 100) / 2)) + 50;
		// sin_arr[i] = ((sin(i * 2 * PI / n) + 1) * (4096 / 2));
	}
}

int ARR_Cal(float freq)  //Calculates the ARR value used for sine-wave generation
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

/*function that sets the ARR value to the timer responsible for sine-wave generation,
we change the ARR value to manipulate the output frequency, the larger the ARR value the lower the frequency as the timer needs more time to reach the ARR value*/
void setARR(float *values, uint8_t n)
{
	int ARR_Val;
	float freq;

	freq = values[n];

	ARR_Val = ARR_Cal(freq);

	if (ARR_Val > 0)
	{
		// TIM2->ARR = ARR_Val; // zapiše vrednost v register
		__HAL_TIM_SET_AUTORELOAD(&htim6, ARR_Val);
		//__HAL_TIM_SET_COUNTER(&htim2, ARR_Val);
	}
	if (ARR_Val <= 0)
	{

		Error_Handler();
	}
}

/*assembleFloat function will combine 4 uint8_t values and merge them together to form a 32-bit float value like it was before sent via HID*/
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
int assembleInt(uint8_t *valArr,uint8_t index){

	uint8_t tempArr[4];
	uint8_t maxInd = index + 4;
	int retVal;

	for (int i = 0; index < maxInd; index++)
	{

		tempArr[i] = valArr[index];
		i++;
	}

	memcpy(&retVal, tempArr, sizeof(retVal));

	return retVal;


}
void debug_print_array(uint8_t *arr, int size) //prints the whole array using one function, is necesairy for debugging purposes
{

	for (int i = 0; i < size; i++)
	{

		debug_printf("%d: %u \r\n", i, arr[i]);
	}

	return;
}

/*function that processes the data recieved via HID and converts it back to float in the case of voltages and frequencies, with the time values it just copies them to a seperate array*/
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

/*this function calculates the prescaler value for the timer responsible for measuring the elapsed time since last state change*/ 
int prescCalc(int* time,int index){  

	int prescVal;
	unsigned long  TIM_CLK = 32 * pow(10, 6);
	unsigned long  ARR = pow(2,32) - 1;


	prescVal= round((double)(TIM_CLK*time[index]*60)/ARR);

	return prescVal + 1;

}
uint32_t TIM_GetCounter(TIM_TypeDef* TIMx){

	/* preveri če so parametri pravilni*/
	/*checks if the parameters are valid*/
	assert_param(IS_TIM_ALL_PERIPH(TIMx)); 

	/*vrne trenutno vrednost števca*/
	/*returns current counter state*/
	return TIMx->CNT;

}
void TIM_resetCounder(TIM_TypeDef* TIMx){

	/* preveri če so parametri pravilni*/
	/*checks if the parameters are valid*/
	assert_param(IS_TIM_ALL_PERIPH(TIMx));

	/*zapiše vrednost 0 v števec*/
	/*writes 0 to the counter value*/
	TIMx->CNT = 0;


}
void TIM_setPrescaler(TIM_TypeDef* TIMx,int val){

	
	/*checks if the parameters are valid*/
	assert_param(IS_TIM_ALL_PERIPH(TIMx));

	
	/*writes 0 to the counter value*/
	TIMx->PSC = val;


}
void setDigiPot(I2C_HandleTypeDef* I2C,float voltage, uint8_t digiPotAddr){  //writes the value to the I2C digital potentiometer //using non inverting amplifier

	float dac_max_voltage = 3.3;
	float A, R1,R2 = 10000;
	uint8_t digipotResolution = 64;
	uint32_t  digipotR = 10000;
	//double step = 156.25;
	uint8_t trainsmitArr[5] = {0};
	uint8_t wiperPos;

	A = voltage/dac_max_voltage;

	R1 = R2/(A-1);

	wiperPos = R1*(digipotResolution/digipotR);

	debug_printf("R1: %d \r\n", R1);


	trainsmitArr[0] = 0;
	trainsmitArr[1] = wiperPos;
	
	HAL_I2C_Master_Transmit(I2C,digiPotAddr  << 1,trainsmitArr,2,100);
	

}
/*this function scans the i2c bus for devices and prints their addresses to the serial console, it is used to check if all 3 i2c devices are connected and functioning*/
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



void writeDataInfoToScreen(char msgString[4][50],float*  float_arr,float* voltage_arr, int* time_arr,int set,uint8_t cursor_pos_x,uint8_t cursor_pos_y){

	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(cursor_pos_x,cursor_pos_y);
	ssd1306_WriteString("current settings:", Font_6x8, White);
	ssd1306_UpdateScreen();
	cursor_pos_y += 10;
	ssd1306_SetCursor(cursor_pos_x,cursor_pos_y);
	sprintf(msgString[0],"frequency: %d",(int)frequencies[set]); // %g ali %f ne dela
	ssd1306_WriteString(msgString[0], Font_6x8, White);
	ssd1306_UpdateScreen();
    cursor_pos_y += 10;
	ssd1306_SetCursor(cursor_pos_x,cursor_pos_y);
	sprintf(msgString[1],"voltage: %d",(int)voltages[set]);
	ssd1306_WriteString(msgString[1], Font_6x8, White);
	ssd1306_UpdateScreen();
	cursor_pos_y += 10;
	ssd1306_SetCursor(cursor_pos_x,cursor_pos_y);
	sprintf(msgString[2],"duration: %d",time[set]);
	ssd1306_WriteString(msgString[2], Font_6x8, White);
	ssd1306_UpdateScreen();
	cursor_pos_y += 10;
	ssd1306_SetCursor(cursor_pos_x,cursor_pos_y);
	sprintf(msgString[3],"current data set: %d",set+1);
	ssd1306_WriteString(msgString[3], Font_6x8, White);
	ssd1306_UpdateScreen();
}
void displayInitData(){
	uint8_t x_pos = 0, y_pos = 0;

	y_pos +=2;

	ssd1306_Fill(Black);
  	ssd1306_SetCursor(x_pos,y_pos);
  	ssd1306_WriteString("LF sine generator", Font_6x8, White);
  	y_pos+=12;
  	ssd1306_SetCursor(x_pos,y_pos);
  	ssd1306_WriteString("By Uros Tomazic", Font_6x8, White);
  	ssd1306_UpdateScreen();


}

void initDigiPot(I2C_HandleTypeDef* i2cx,uint8_t device_addr){

	uint8_t data_arr[4] = {0};

	data_arr[0] = 0;
	data_arr[1] = 0x40;  //sets the digipot to 10kR (max value) so we get a gain of 1 at the output


	HAL_I2C_Master_Transmit(i2cx,device_addr,data_arr,2,100);
	
}

void EEPROM_Write (I2C_HandleTypeDef* i2cx,uint16_t eeprom_addr,uint16_t page, uint16_t offset, uint8_t *data, uint16_t size){

	int paddrposition = log(PAGE_SIZE)/log(2);

	uint16_t startPage = page;
	uint16_t endPage = page + ((size+offset)/PAGE_SIZE);

	uint16_t numofpages = (endPage-startPage) + 1;
	uint16_t pos=0;

	for (int i=0; i<numofpages; i++){

		uint16_t MemAddress = startPage<<paddrposition | offset;
		uint16_t bytesremaining = bytestowrite(size, offset);  

		HAL_I2C_Mem_Write(i2cx,eeprom_addr, MemAddress, 2, &data[pos], bytesremaining, 1000);

		startPage += 1;
		offset=0;
		size = size-bytesremaining; 
		pos += bytesremaining;

		HAL_Delay (5); //5ms delay for stability reasons

	}


}

void EEPROM_Read (I2C_HandleTypeDef* i2cx,uint16_t eeprom_addr,uint16_t page, uint16_t offset, uint8_t *data, uint16_t size){

		int paddrposition = log(PAGE_SIZE)/log(2);

		uint16_t startPage = page;
		uint16_t endPage = page + ((size+offset)/PAGE_SIZE);

		uint16_t numofpages = (endPage-startPage)+1;
		uint16_t pos=0;

		for (int i=0; i<numofpages; i++){

		uint16_t MemAddress = startPage<<paddrposition | offset;
		uint16_t bytesremaining = bytestowrite(size, offset);
		HAL_I2C_Mem_Read(i2cx, eeprom_addr, MemAddress, 2, &data[pos], bytesremaining, 1000);
		startPage += 1;
		offset=0;
		size = size-bytesremaining;
		pos += bytesremaining;



		}


}
void EEPROM_PageErase (I2C_HandleTypeDef* i2cx,uint8_t eeprom_addr,uint16_t page){

		int paddrposition = log(PAGE_SIZE)/log(2);
		uint16_t MemAddress = page<<paddrposition;

		uint8_t data[PAGE_SIZE];
		memset(data,0xff,PAGE_SIZE);
		HAL_I2C_Mem_Write(i2cx, EEPROM_ADDR, MemAddress, 2, data, PAGE_SIZE, 1000);

		HAL_Delay (5);


}
uint16_t bytestowrite (uint16_t size, uint16_t offset)
{
	if ((size+offset)<PAGE_SIZE) return size;

	else return PAGE_SIZE-offset;
}
void split_data_int(uint8_t* dataArr, int num){

	uint8_t *p;
	p = (uint8_t*) &num;

	for(int i = 0;i <sizeof(int);i++){

		dataArr[i] = *p;

		p++;
	}

}
void split_data_float(uint8_t* dataArr, float num){

	uint8_t *p;
	p = (uint8_t*) &num;

	for(int i = 0;i <sizeof(float);i++){

		dataArr[i] = *p;

		p++;
	}

}
void savePreset(float *floatArr,float *voltageArr,int *timeArr,I2C_HandleTypeDef *hi2cx,uint16_t eeprom_addr){

	uint8_t arr[128] = {0},tempArr[4] = {0};
	uint8_t currentPos = 0;
	uint8_t next = currentPos + 4;
	
	


	for(int i = 0; i < 10; i++){
		
		split_data_float(tempArr,floatArr[i]);
		for(int j = 0;currentPos < next;currentPos++,j++){

			arr[currentPos] = tempArr[j];

		}
		next = currentPos + sizeof(float);
	}
	for(int i = 0; i < 10; i++){
		
		split_data_float(tempArr,voltageArr[i]);
		for(int j = 0;currentPos < next;currentPos++,j++){

			arr[currentPos] = tempArr[j];

		}
		next = currentPos + sizeof(float);
	}
		for(int i = 0; i < 10; i++){
		
		split_data_int(tempArr,timeArr[i]);
		for(int j = 0;currentPos < next;currentPos++,j++){

			arr[currentPos] = tempArr[j];

		}
		next = currentPos + sizeof(int);
	}	

	debug_printf("data write to eeprom: \r\n");
	debug_print_array(arr,128);
	EEPROM_Write (hi2cx,eeprom_addr, 0, 0, arr, 128);

}

void readPreset(float *floatArr,float *voltageArr,int *timeArr,I2C_HandleTypeDef *hi2cx,uint16_t eeprom_addr){

	uint8_t arr[128] = {0};
	uint8_t currentIndex = 0;
	debug_printf("read array from eeprom: \r\n");
	debug_print_array(arr,128);

	EEPROM_Read(hi2cx,eeprom_addr,0, 0, arr, 128);

	for(int i = 0;i < 10; i++,currentIndex += 4){

		floatArr[i] = assembleFloat(arr,currentIndex);
		
	}
	for(int i = 0; i < 10; i++,currentIndex += 4){


		voltageArr[i] = assembleFloat(arr,currentIndex);

	}
	for(int i = 0; i < 10; i++,currentIndex += 4){

		timeArr[i] = assembleInt(arr,currentIndex);

	}
	

}