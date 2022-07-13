/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
// status leds are PB13 (built in led on the board), PB2 external red error led, PC7 external status led (no use yet)
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dac.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "stdarg.h"
#include "stdio.h"
#include "usbd_custom_hid_if.h"
#include "functions.h"
#include "string.h"
#include "ssd1306.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */



uint32_t sin_out[1000];
uint32_t arr_len = 255;
uint32_t elapsedTime = 0;
uint8_t cursor_pos_x = 0;
uint8_t cursor_pos_y = 0;
char msgString[4][50] = {0};
#define PI 3.141592653

extern float frequencies[10] = {0};
extern float voltages[10] = {0};
extern int time[10] = {0};
extern int processState = 0;
int currentSet = 0;
uint32_t curtime = 0,curtime2 = 0;





uint8_t dataCurrentState = NO_DATA; 
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DAC1_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  ssd1306_Init();
  /* USER CODE BEGIN 2 */

  //HAL_TIM_Base_Start(&htim2);    // tim for delay 32 bit
  HAL_TIM_Base_Start(&htim6); // tim for dac 16bit

  calcsin(sin_out, 3.3);
  status = 0;
  HAL_DAC_Start_DMA(&hdac1, DAC1_CHANNEL_1, (uint32_t *)sin_out, arr_len, DAC_ALIGN_12B_R); // starts the DAC with DMA reading data from sin_out array

  debugI2Cscan(&hi2c1,&huart2);

 

  debug_printf("successful init \r\n");

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1); // vgrajena zelena ledica na plošči, sporoči uspešno inicializacijo
  ssd1306_Fill(Black);
  cursor_pos_y+=2;
  ssd1306_SetCursor(cursor_pos_x,cursor_pos_y);
  ssd1306_WriteString("LF sine generator", Font_6x8, White);
  cursor_pos_y+=12;
  ssd1306_SetCursor(cursor_pos_x,cursor_pos_y);
  ssd1306_WriteString("By Uros Tomazic", Font_6x8, White);
  ssd1306_UpdateScreen();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    if (status == 1)
    {
     // debug_printf("entered state 0:%d, 1:%d \r\n", buffer[0], buffer[1]); //debug information

      if (buffer[1] == 1)
      {

        memcpy((uint8_t *)buffer1, (uint8_t *)buffer, 64);
        USBD_free(buffer);

        debug_printf("recieved 1st array \r\n");
        debug_print_array(buffer1, 64);
        
      }
      if (buffer[1] == 2)
      {

        memcpy((uint8_t *)buffer2, (uint8_t *)buffer, 64);
        USBD_free(buffer);
        debug_printf("recieved 2nd array \r\n");
        debug_print_array(buffer2, 64);


        printf("process data \r\n");
        processData(frequencies, voltages, time, buffer1, buffer2); //converts data from int to float and writes it to 
        dataCurrentState = RECIEVED_FROM_USB;
        processState = 0;
        /*debug information*/  
        debug_printf("frequencies \r\n");
        for(int i = 0;i < 10;i++){debug_printf("%d: %d \r\n",i,(int)frequencies[i]);}
        debug_printf("voltages \r\n");
        for(int i = 0;i < 10;i++){debug_printf("%d: %d \r\n",i,(int)voltages[i]);}
        debug_printf("time \r\n");
        for(int i = 0;i < 10;i++){debug_printf("%d: %d \r\n",i,(int)time[i]);}
    


      }  

   
      status = 0;
    }
   
    if(processState == 1 && (dataCurrentState == READ_FROM_MEMORY || dataCurrentState == RECIEVED_FROM_USB) ){
             
      setARR(frequencies,currentSet);  
      cursor_pos_x = 2; 
      cursor_pos_y = 0;
    	writeDataInfoToScreen(msgString,frequencies, voltages,  time, currentSet, cursor_pos_x, cursor_pos_y);
      processState = 2;
      curtime = HAL_GetTick();
      
      
    }

    if((HAL_GetTick() > ((time[currentSet]*60*1000) + curtime)) && processState == 2){ //time[currentSet]*60*1000000
      
      processState = 1; 
      currentSet++;
      debug_printf("current set finished, moving on to the next one \r\n");    

      }

      if(currentSet >= 10 && processState != 100){
        processState = 100;
        ssd1306_Fill(Black);
	      ssd1306_UpdateScreen();
        ssd1306_SetCursor(0,32);
	      ssd1306_WriteString("process finished", Font_6x8, White);
	      ssd1306_UpdateScreen();

      }



    curtime2 = HAL_GetTick();
    HAL_Delay(100); // added delay for stability reasons
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure LSE Drive Capability
   */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration
   */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */
void debug_printf(const char *format, ...)
{
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
   while (HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY){}
  // tukaj pokliči UART_SEND kjer pošlješ buffer z dolžino strlen(buffer)
  va_end(args);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2)
  {
    debug_printf("timer interrupt\r\n");
    //processState = 3;
    
  }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

  if(GPIO_Pin == GPIO_PIN_13){
    debug_printf("button pressed\r\n");
    if(processState != 2 && dataCurrentState != NO_DATA ){
      
      processState = 1;
    
    }
  }
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    debug_printf("error \r\n");
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 1); // error led 1
    HAL_Delay(100);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
