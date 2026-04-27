/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
void Error_Handler(void);

/**
  * @brief  The application entry point.
  */
int main(void)
{
  /* 1. Ініціалізація HAL */
  HAL_Init();

  /* 2. Налаштування системи тактування (64 МГц) */
  SystemClock_Config();

  /* 3. Ініціалізація периферії (GPIO та ADC) */
  MX_GPIO_Init();
  MX_ADC1_Init();

  /* 4. КАЛІБРУВАННЯ АЦП (Критично важливо для серії F103) */
  HAL_ADCEx_Calibration_Start(&hadc1);

  uint32_t adcValue = 0;
  float voltage = 0.0f;

  while (1)
  {
      HAL_ADC_Start(&hadc1);

      if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
      {
          uint32_t rawValue = HAL_ADC_GetValue(&hadc1);
          float v = ((float)rawValue * 3.3f) / 4095.0f;

          // Якщо напруга більше 1.5В — діод світить постійно
          if (v > 1.5f)
          {
              HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
          }
          else
          {
              // Якщо менше 1.5В — діод блимає.
              // Чим менша напруга, тим повільніше блимання.
              HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

              // Робимо затримку пропорційну значенню (мінімум 50мс)
              uint32_t delay = (rawValue / 10) + 50;
              HAL_Delay(delay);
          }
      }
      else
      {
          // Якщо АЦП взагалі не відповідає — діод буде "тьмяно" мерехтіти
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
          HAL_Delay(5);
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
          HAL_Delay(5);
      }
  }
}

/**
  * @brief System Clock Configuration (64 MHz)
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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

/**
  * @brief ADC1 Initialization (Channel 1, PA1)
  */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1; // Порядковий номер у списку опитування
  sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization (PC13)
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Початковий стан: вимкнено */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**
  * @brief Обробник критичних помилок
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
