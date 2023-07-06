/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LOW 1000
#define HIGH 3000
#define LISTSIZE 10
#define MAXTIME 100000
#define MAXSPHYFMUS 150
#define MINSPHYFMUS 50
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t time;				 // 当前时间
uint32_t l_time;
uint16_t adc_value;		 // adc采样值
uint16_t last_value;	 // 上个adc采样值
uint32_t delta_time_list[LISTSIZE];  // 脉搏间的时间间隔列表
int alarm;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 串口输出
int fputc(int ch,FILE *f)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)&ch,1,10);
	return ch;
}

// adc采样
void adc_sample()
{
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1,10);
}

// 检测上升沿
int check_posiedge()
{
	if(adc_value>HIGH&&last_value<HIGH)
		return 1;
	return 0;
}

// 检测下降沿
int check_negedge()
{
	if(adc_value<HIGH&&last_value>HIGH)
		return 1;
	return 0;
}

// 求时间间隔平均值
double average()
{
	uint32_t sum=0;
	for(uint8_t i=0;i<LISTSIZE;i++)
	{
		sum+=delta_time_list[i];
	}
	return (float)sum/LISTSIZE;
}

//向队列中添加新的时间间隔
void add_delta_time()
{
	uint32_t delta_time;
	if(time<l_time)
	{
		delta_time=MAXTIME-l_time+time;
	}
	else
	{
		delta_time=time-l_time;
	}
	
	for(int i=0;i<LISTSIZE-1;i++)
	{
		delta_time_list[i]=delta_time_list[i+1];
	}
	delta_time_list[LISTSIZE-1] = delta_time;
}

//计算心率
double get_sphygmus()
{
	double avg_time=average();
	double sphygmus=(double)MAXTIME/avg_time*60;
	alarm=(sphygmus>MAXSPHYFMUS||sphygmus<MINSPHYFMUS);

	return sphygmus;
}
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
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_ADC_Start_IT(&hadc1);
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_ADCEx_Calibration_Start(&hadc1); 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		HAL_ADC_Start_IT(&hadc1);
//		printf("%f\r\n",get_sphygmus());
		printf("xxxxx\r\n");

		HAL_Delay(100);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//adc中断
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	last_value=adc_value;
	adc_value=HAL_ADC_GetValue(&hadc1);
}



//时钟中断
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	time++;
	if(time>MAXTIME)
	{
		HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_0);
		time=0;
	}
}	

//外部中断
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if((GPIO_Pin&GPIO_PIN_8))
	{
		add_delta_time();
		l_time=time;
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
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
