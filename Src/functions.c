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

	if (ARR_Val > 0) // if the value is above 0 it is safe to write to the arr register
	{
		// TIM2->ARR = ARR_Val; // zapiše vrednost v register
		__HAL_TIM_SET_AUTORELOAD(&htim6, ARR_Val);
		//__HAL_TIM_SET_COUNTER(&htim2, ARR_Val);
	}
	if (ARR_Val <= 0)  // if ARR value is below 0 it was not calculated with the right data and could cause errors if written to the ARR register
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
/* assembleInt function will combine one integer from 4 uint8_t values (this is basically an unsigned char) */
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
void debug_print_array(uint8_t *arr, int size) //prints the whole array using one function,makes debugging easier
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

	
	/*checks if the parameters are valid*/
	assert_param(IS_TIM_ALL_PERIPH(TIMx)); 

	/*returns current counter state*/
	return TIMx->CNT;

}
void TIM_resetCounder(TIM_TypeDef* TIMx){

	
	/*checks if the parameters are valid*/
	assert_param(IS_TIM_ALL_PERIPH(TIMx));

	/*writes 0 to the counter value*/
	TIMx->CNT = 0;


}
void TIM_setPrescaler(TIM_TypeDef* TIMx,int val){

	
	/*checks if the parameters are valid*/
	assert_param(IS_TIM_ALL_PERIPH(TIMx));

	
	/*writes prescaler value*/
	TIMx->PSC = val;


}


void setDigiPot(I2C_HandleTypeDef* I2C,float voltage, uint8_t digiPotAddr){  //writes the value to the I2C digital potentiometer //using non inverting amplifier

	float dac_max_voltage = 3.3; // maximum voltage that the digital to analog converter can output
	float A, R1,R2 = 10000;
	uint8_t digipotResolution = 64; // resolution of the digital potentiometer 
	uint32_t  digipotR = 10000;  // whole resistance of the digital potentiometer
	uint8_t trainsmitArr[5] = {0}; 
	uint8_t wiperPos;

	A = voltage/dac_max_voltage; // gain caluclation

	R1 = R2/(A-1); // calculate the resistance we need to set the digipot to

	wiperPos = R1*(digipotResolution/digipotR); // calculate to which position we need to set the internal wiper of the digipot

	debug_printf("R1: %d \r\n", R1);


	trainsmitArr[0] = 0;
	trainsmitArr[1] = wiperPos;
	
	HAL_I2C_Master_Transmit(I2C,digiPotAddr,trainsmitArr,2,100); //writes the calculated wiperpos to the digipot via I2C
	

}
void setDigiPot2(I2C_HandleTypeDef* I2C,float voltage, uint8_t digiPotAddr){

	float dac_voltage, A_amp = 4, digipot_R = 10000, digipot_current,R2_voltage,R1,R2; // the 10k value needs to be later adjusted with the included wiper resistance
	uint8_t digipotResolution = 64;
	uint8_t wiperPos;
	uint8_t data_arr[5] = {0};

	dac_voltage = 3.3;  

	digipot_current =  dac_voltage / digipot_R;

	R2_voltage = voltage/A_amp;

	R2 = R2_voltage/digipot_current;

	R1 = digipot_R - R2;

	wiperPos = R1*(digipotResolution/digipot_R);

	data_arr[0] = 0;
	data_arr[1] = wiperPos;


	HAL_I2C_Master_Transmit(I2C,digiPotAddr,data_arr,2,100);


	


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


/* write current settings that the generator is using to produce the sine wave, where frequency is the frequency of the sine wave, 
voltage is the peak voltage of the sine wave and time is the duraton untill we switch to the next set of data*/
void writeDataInfoToScreen(char msgString[4][50],float*  float_arr,float* voltage_arr, int* time_arr,int set,uint8_t cursor_pos_x,uint8_t cursor_pos_y){

	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
	ssd1306_SetCursor(cursor_pos_x,cursor_pos_y);
	ssd1306_WriteString("current settings:", Font_6x8, White);
	ssd1306_UpdateScreen();
	cursor_pos_y += 10;
	ssd1306_SetCursor(cursor_pos_x,cursor_pos_y);
	sprintf(msgString[0],"frequency: %d",(int)frequencies[set]); // %g or %f doesn't work yet, so we just display the value without decimals for now
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

/* writes the name of the device and author to the OLED screen so it's not blank before we start generating the sine wave*/
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
/* sets the resistance of the digipot to 10kR so we get a gain of 1 or 3.3V at the output*/
void initDigiPot(I2C_HandleTypeDef* i2cx,uint8_t device_addr){

	uint8_t data_arr[4] = {0};

	data_arr[0] = 0;
	data_arr[1] = 0x40;  //sets the digipot to 10kR (max value) so we get a gain of 1 at the output


	HAL_I2C_Master_Transmit(i2cx,device_addr,data_arr,2,100);
	
}

/* function to write provided data array to the attached I2C EEPROM chip, as of now there is a bug somewhere which doesn't allow writing or possibly reading to/from the ic*/

void EEPROM_Write(I2C_HandleTypeDef* i2cx,uint8_t eeprom_addr,uint16_t page, uint16_t offset, uint8_t *data, uint16_t size){

uint8_t transmit_arr[20] = {0},cur_page = page;



//HAL_I2C_Mem_Write(i2cx,eeprom_addr, 8, 16, p, 128, 100);
//transmit_arr[0] = cur_page;
for(int i = 0; i < 8; i++){ // maybe i+=2
	for(int j = i*16, k = 0; k<16;j++,k++){
		
		transmit_arr[k] = data[j];

		
	}
	HAL_I2C_Mem_Write(i2cx,eeprom_addr,cur_page,I2C_MEMADD_SIZE_8BIT,transmit_arr,16,100);
	while(HAL_I2C_GetState(i2cx)!= HAL_I2C_STATE_READY){}
	//HAL_I2C_Master_Transmit(i2cx,eeprom_addr,transmit_arr,17,100);
	cur_page++;


}

}

void EEPROM_Read (I2C_HandleTypeDef* i2cx,uint8_t eeprom_addr,uint16_t page, uint16_t offset, uint8_t *data, uint16_t size){

	
	uint8_t recieve_arr[20] = {0},cur_page = page;

	for(int i = 0; i < 8; i++){ // maybe i+=2
	
	HAL_I2C_Mem_Read(i2cx,eeprom_addr,cur_page,I2C_MEMADD_SIZE_8BIT,recieve_arr,16,100);
	while(HAL_I2C_GetState(i2cx)!= HAL_I2C_STATE_READY){}
	
		for(int j = i*16,k = 0;k < 16;j++,k++){

			data[j] = recieve_arr[k];

		}


		cur_page++;
	}



}
void EEPROM_PageErase (I2C_HandleTypeDef* i2cx,uint8_t eeprom_addr,uint16_t page){

		int paddrposition = log(PAGE_SIZE)/log(2);
		uint16_t MemAddress = page<<paddrposition;

		uint8_t data[PAGE_SIZE];
		memset(data,0xff,PAGE_SIZE);
		HAL_I2C_Mem_Write(i2cx,eeprom_addr, MemAddress, 2, data, PAGE_SIZE, 1000);

		HAL_Delay (5);


}
uint16_t bytestowrite (uint16_t size, uint16_t offset) /*sub function to  EERPOM write, which is used to calculate how many bytes we still need to write to the EEPROM*/
{
	if ((size+offset)<PAGE_SIZE) return size;

	else return PAGE_SIZE-offset;
}

/* splits a INT which is 4 bytes to 4 uint8_t numbers (also known as unsigned char) so we can write it to the EEPROM or send it via custom hid protocol (USB)*/
void split_data_int(uint8_t* dataArr, int num){

	uint8_t *p;
	p = (uint8_t*) &num;

	for(int i = 0;i <sizeof(int);i++){

		dataArr[i] = *p;

		p++;
	}

}
/* splits a float which is 4 bytes to 4 uint8_t numbers (also known as unsigned char) so we can write it to the EEPROM or send it via custom hid protocol (USB)*/
void split_data_float(uint8_t* dataArr, float num){

	uint8_t *p;
	p = (uint8_t*) &num;

	for(int i = 0;i <sizeof(float);i++){

		dataArr[i] = *p;

		p++;
	}

}

/* this function saves the data it recieved via USB to an external eeprom or internal flash, depends on which function is called below*/
void savePreset(float *floatArr,float *voltageArr,int *timeArr,I2C_HandleTypeDef *hi2cx,uint16_t eeprom_addr){

	uint8_t arr[128] = {0},tempArr[4] = {0};
	uint8_t currentPos = 0;
	uint8_t next = currentPos + 4;
	
	


	for(int i = 0; i < 10; i++){
		
		split_data_float(tempArr,floatArr[i]);
		for(int j = 0;currentPos < next;currentPos++,j++){

			arr[currentPos] = tempArr[j];

		}
		next = currentPos + 4;
	}
	for(int i = 0; i < 10; i++){
		
		split_data_float(tempArr,voltageArr[i]);
		for(int j = 0;currentPos < next;currentPos++,j++){

			arr[currentPos] = tempArr[j];

		}
		next = currentPos + 4;
	}
		for(int i = 0; i < 10; i++){
		
		split_data_int(tempArr,timeArr[i]);
		for(int j = 0;currentPos < next;currentPos++,j++){

			arr[currentPos] = tempArr[j];

		}
		next = currentPos + 4;
	}	

	debug_printf("data write to eeprom: \r\n");
	debug_print_array(arr,128);
	HAL_Delay(10);
	EEPROM_Write(hi2cx,eeprom_addr,8, 0, arr, 128);
	//writeToFlash(arr);
}
/*reads data from an external EEPROM or internal flash, depends on which function is called*/
void readPreset(float *floatArr,float *voltageArr,int *timeArr,I2C_HandleTypeDef *hi2cx,uint16_t eeprom_addr){

	uint8_t arr[128] = {0};
	uint8_t currentIndex = 0;


	EEPROM_Read(hi2cx,eeprom_addr,8, 0, arr, 128);

	//readFromFlash(arr);

	debug_printf("read array from eeprom: \r\n");
	debug_print_array(arr,128);

	for(int i = 0;i < 10; i++){

		floatArr[i] = assembleFloat(arr,currentIndex);
		currentIndex +=4;
		
	}
	for(int i = 0; i < 10; i++){


		voltageArr[i] = assembleFloat(arr,currentIndex);
		currentIndex +=4;

	}
	for(int i = 0; i < 10; i++){

		timeArr[i] = assembleInt(arr,currentIndex);
		currentIndex +=4;

	}
	   debug_printf("frequencies \r\n");
        for(int i = 0;i < 10;i++){debug_printf("%d: %d \r\n",i,(int)frequencies[i]);}
        debug_printf("voltages \r\n");
        for(int i = 0;i < 10;i++){debug_printf("%d: %d \r\n",i,(int)voltages[i]);}
        debug_printf("time \r\n");
        for(int i = 0;i < 10;i++){debug_printf("%d: %d \r\n",i,(int)time[i]);}

}
/*writes a provided array to internal flash of the microcontroler*/
void writeToFlash(uint8_t* arr){



	uint32_t pageAddress = 0x0803F800;
	uint64_t data[16] = {0};

	memcpy((uint8_t*)data,(uint8_t*) arr,128);

	HAL_FLASH_Unlock();
	
	
	for(int i = 0; i < 16; i++ ){

			
				
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,pageAddress,data[i]);
		
		pageAddress+=8;
		
	}

	HAL_FLASH_Lock();


}
/*reads 128 bytes of data from internal flash on the set page adress and stores it to the provided array*/
void readFromFlash(uint8_t* arr){

	uint32_t pageAddress = 0x0803F800; //flash block 127 (the last one)
	uint64_t dataArr[16]= {0};	
	HAL_FLASH_Unlock();

	for(int i = 0; i < 16; i++,pageAddress+=8){

		dataArr[i] = *(__IO uint32_t *)pageAddress;

	}

	HAL_FLASH_Lock();

	memcpy((uint8_t*)arr,(uint8_t*)dataArr,128);



	
}


