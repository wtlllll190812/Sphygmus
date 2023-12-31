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
#include "oled.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// 时间差队列相关
#define MINLISTSIZE 15 // 最小列表长度 (超过该值视为心率开始稳定，开启报警)
#define MAXLISTSIZE 60 // 最大列表长度 (维护时间差队列的长度)

// 告警相关
#define MAXSPHYFMUS 150 // 告警心率上阈值
#define MINSPHYFMUS 90  // 告警心率下阈值

// 计时器相关
#define MAXTIME 100000 // 计时器的最大值
#define UART1TIME 20   // 串口1发送时间间隔
#define UART2TIME 2000 // 串口2发送时间间隔
#define ADC1TIME 20    // adc采样时间间隔
#define CLOSETIME 5000 // 关机时间间隔

// 数据过滤相关
#define MIN_DELTA 333       // 最小时间间隔
#define MAX_DELTA 1000      // 最大时间间隔
#define MAX_DELTA_DELTA 250 // 最大时间间隔变化量
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

int too_high; // 报警标志
int too_low;  // 报警标志

int is_open = 0;   // 是否开机
int is_closed = 0; // 是否已经关机

int heart_beat = 0; // 心跳标志

uint8_t sphygmus_num; // 测试到的心跳次数
uint16_t adc_value;   // adc采样值
uint16_t last_value;  // 上个adc采样值
double sphygmus;      // 脉搏

uint32_t time;   // 当前时间
uint32_t l_time; // 上次心跳时间

uint32_t delta_time_list[MAXLISTSIZE]; // 脉搏间的时间间隔列表
UART_HandleTypeDef current_uart;       // 当前使用的串口

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// 初始化
void init(void);

// 获取时间间隔
uint32_t get_delta_time(void);

// 微妙级延时
void delay_us(uint32_t nus);

// 输出
int fputc(int ch, FILE *f);

// adc采样
void adc_sample(void);

// 脉搏间隔平均值
double average(void);

// 添加时间间隔
void add_delta_time(void);

// 获取脉搏
void get_sphygmus(void);

// 数字转换为字符串
char *to_string(uint32_t num, char *str, int size);

// oled显示
void display(void);

// 报警
void give_alarm(void);

// 关机
void on_close(void);

// 开机
void start(void);
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
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  init();      // 初始化系统
  is_open = 0; // 默认关闭
  HAL_ADC_Start_IT(&hadc1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (is_open) // 若处于开机状态
    {
      display();      // oled显示
      give_alarm();   // 满足条件时报警
      get_sphygmus(); // 计算当前心率
    }
    else if (!is_closed) // 若退出开机状态且还没有完成关机
    {
      HAL_GPIO_WritePin(GPIOC, Alarm_Pin, GPIO_PIN_SET); // 关闭蜂鸣器
      OLED_Clear(0x00);                                  // oled清屏
      sphygmus_num = 0;                                  // 清除心跳计数
      is_closed = 1;                                     // 完成关机
    }
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// adc中断
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  last_value = adc_value;
  adc_value = HAL_ADC_GetValue(&hadc1); // 获取adc值
}

// 定时器中断
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == (&htim1))
  {
    time++; // 计时
    if (time > MAXTIME)
    {
      time = 0;
    }
    if (is_open)
    {
      // adc测量一次
      if (time % ADC1TIME == 0)
      {
        HAL_ADC_Start_IT(&hadc1);
      }

      // 串口1输出
      if (time % UART1TIME == 0)
      {
        current_uart = huart1;
        printf("%d %f\r\n", adc_value, sphygmus);
      }

      // 串口2输出
      if (time % UART2TIME == 0)
      {
        current_uart = huart2;
        printf("%f\r\n", sphygmus);
      }

      // 超过设置时间时关机
      if (get_delta_time() > CLOSETIME)
      {
        on_close();
      }
    }
  }
}

// 外部中断（用于测量脉搏）
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin & GPIO_PIN_11)
  {
    if (!is_open && sphygmus_num > 2) // 接收到两次以上的脉搏信号后，系统开机
    {
      start(); // 开机初始化
      l_time = time;
      sphygmus_num = 0;
    }

    add_delta_time(); // 向队列添加新的时间间隔
    heart_beat = 1;   // 心跳标志置1
    l_time = time;    // 记录上一次脉搏的时间
  }
}

// 初始化
void init()
{
  // 初始化变量
  for (int i = 0; i < MAXLISTSIZE; i++)
  {
    delta_time_list[i] = 0;
  }

  current_uart = huart1;
  Oled_Init();
  sphygmus_num = 0;

  // 开启adc和计时器
  HAL_ADC_Start_IT(&hadc1);
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_ADCEx_Calibration_Start(&hadc1);
}

// 计算时间间隔
uint32_t get_delta_time()
{
  uint32_t delta_time;
  // 时间溢出，需要特殊处理
  if (time < l_time)
  {
    delta_time = MAXTIME - l_time + time;
  }
  else
  {
    delta_time = time - l_time;
  }
  return delta_time;
}

// 微妙级延时
void delay_us(uint32_t nus)
{
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  __HAL_TIM_ENABLE(&htim2);
  while (__HAL_TIM_GET_COUNTER(&htim2) < nus)
  {
  }
  __HAL_TIM_DISABLE(&htim2);
}

// oled显示
void display()
{
  static char buf[] = {"rate: "};
  static char buf1[] = {"rate:"};
  static char sphygmus_str[5];

  // 显示心率
  Oled_Display_String(0, 80, buf1);

  // 显示心脏图片
  Oled_Display_Pic(50, 50, 0, 15, heart_small);
  Oled_Display_String(0, 80, buf);
  Oled_Display_String(3, 80, to_string(sphygmus, sphygmus_str, 5));

  // 播放心跳动画
  if (heart_beat)
  {
    heart_beat = 0;
    Oled_Display_Pic(50, 50, 0, 15, heart_large);
  }
}

// 报警
void give_alarm()
{
  static char low[] = {"too low "};
  static char high[] = {"too high"};
  static char normal[] = {"        "};

  // 超过最大值或者低于最小值时报警
  if (too_high && is_open)
  {
    HAL_GPIO_WritePin(GPIOB, Alarm_Pin, GPIO_PIN_SET);
    Oled_Display_String(6, 10, high);
  }
  else if (too_low && is_open)
  {
    HAL_GPIO_WritePin(GPIOB, Alarm_Pin, GPIO_PIN_SET);
    Oled_Display_String(6, 10, low);
  }
  else
  {
    HAL_GPIO_WritePin(GPIOB, Alarm_Pin, GPIO_PIN_RESET);
    Oled_Display_String(6, 10, normal);
  }
}

// 关机
void on_close()
{
  is_open = 0;
  is_closed = 0;
}

// 开机
void start()
{
  is_open = 1;
  for (int i = 0; i < MAXLISTSIZE; i++)
  {
    delta_time_list[i] = 0;
  }
  // Oled_Init();
}

// 输出
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&current_uart, (uint8_t *)&ch, 1, 10);
  return ch;
}

// adc采样
void adc_sample()
{
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, 10);
}

// 脉搏间隔平均值
double average()
{
  uint32_t sum = 0;
  for (uint8_t i = 0; i < MAXLISTSIZE; i++)
  {
    sum += delta_time_list[i];
  }
  int num = sphygmus_num > MAXLISTSIZE ? MAXLISTSIZE : sphygmus_num;
  return (float)sum / num;
}

// 获取脉搏
void get_sphygmus()
{
  double avg_time = average();
  double sp = 1000.0 / avg_time * 60;

  // 判断是否超过报警阈值
  if (sphygmus_num > MINLISTSIZE)
  {
    too_high = (sp > MAXSPHYFMUS);
    too_low = (sp < MINSPHYFMUS);
  }
  sphygmus = sp;
}

// 添加时间间隔
void add_delta_time()
{
  uint32_t delta_time = get_delta_time(); // 获取时间间隔

  // 过滤掉间隔过短和过长的脉搏
  if (delta_time < MIN_DELTA || delta_time > MAX_DELTA)
  {
    return;
  }

  // 过滤掉间隔不稳定的脉搏
  int delta_delta = delta_time > delta_time_list[MAXLISTSIZE - 1] ? delta_time - delta_time_list[MAXLISTSIZE - 1] : delta_time_list[MAXLISTSIZE - 1] - delta_time;
  if (sphygmus_num >= MINLISTSIZE && delta_delta > MAX_DELTA_DELTA)
  {
    return;
  }

  // 队列元素左移一位
  for (int i = 0; i < MAXLISTSIZE - 1; i++)
  {
    delta_time_list[i] = delta_time_list[i + 1];
  }

  // 添加新的时间间隔
  delta_time_list[MAXLISTSIZE - 1] = delta_time;

  // 脉搏数加一
  sphygmus_num++;
}

// 数字转换为字符串
char *to_string(uint32_t num, char *str, int size)
{
  static char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.";

  int i = 0, j, k;

  do
  {
    str[i++] = index[num % 10];
    num /= 10;

  } while (num && i < size);
  for (; i < size - 1; i++)
  {
    str[i] = ' ';
  }

  str[i] = '\0';

  if (str[0] == '-')
    k = 1;
  else
    k = 0;

  char temp;
  for (j = k; j <= (i - 1) / 2; j++)
  {
    temp = str[j];
    str[j] = str[i - 1 + k - j];
    str[i - 1 + k - j] = temp;
  }

  return str;
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
