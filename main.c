/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "cacheaxi.h"
#include "csi.h"
#include "dcmipp.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "xspi.h"
#include "xspim.h"
#include "gpio.h"
#include "vl53l9.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "utils.h"
#include "semperflash_drv.h"
#include "interface.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SECTORS_COUNT 100
#define MEMORY_SECTOR_SIZE 256  // 4KB sector size for SemperFlash
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
ISP_HandleTypeDef  hcamera_isp;

uint8_t csi_control = 0;
static __IO uint32_t NbMainFrames = 0;

__attribute__((section(".dcmipp_framebuffer")))
__attribute__((aligned(32)))
uint8_t frame_buffer0[FRAME_BYTES];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemIsolation_Config(void);
static void set_risaf_default(RISAF_TypeDef *risaf);
void RISAF_Config(void);
void system_init_post(void);
static uint32_t get_risaf_max_addr(RISAF_TypeDef *risaf);
/* USER CODE BEGIN PFP */
extern void vl53l9_app(void);
void SystemClock_Config(void);
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
  HAL_Init();

  /* USER CODE BEGIN Init */
  system_init_post();
  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

  MX_GPIO_Init();
  MX_CACHEAXI_Init();
  MX_DCMIPP_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C4_Init();
  MX_UART5_Init();
  MX_UART7_Init();
  MX_XSPI1_Init();
  MX_XSPI2_Init();
  SystemIsolation_Config();
  /* USER CODE BEGIN 2 */
  RISAF_Config();
  uint32_t buffer_addr = (uint32_t)frame_buffer0;
  memset((void *)buffer_addr, 0x00, FRAME_BYTES);

  if(csi_control == 0)
  {
    HAL_GPIO_WritePin(CSI_SEL_GPIO_Port, CSI_SEL_Pin, RESET);
    HAL_Delay(200);
    CMW_CameraInit_t camera_init = {
      .width = FRAME_W,
      .height = FRAME_H,
      .fps = 3,
      .pixel_format = 1,
      .mirror_flip = 0
    };

    HAL_GPIO_WritePin(XSHUTDOWN_GPIO_Port, XSHUTDOWN_Pin, RESET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(XSHUTDOWN_GPIO_Port, XSHUTDOWN_Pin, SET);
    HAL_Delay(200);
    
    uint8_t ret = 0;
    ret = CMW_CAMERA_Init(&camera_init);
    if(ret != CMW_ERROR_NONE){
      printf("[Camera Init Error \r\n]");
    }

    while(1)
    {
//    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)BUFFER_ADDRESS_0, FRAME_BYTES);
      ret = CMW_CAMERA_Start(DCMIPP_PIPE1, frame_buffer0, DCMIPP_MODE_SNAPSHOT);
      if (ret != CMW_ERROR_NONE) {
        printf("[Camera Catch Error \r\n]");
      }
//	  SCB_InvalidateDCache_by_Addr((uint32_t *)BUFFER_ADDRESS_0, FRAME_BYTES);
	    HAL_Delay(100);
    }
  }

  HAL_Delay(100);

  if(csi_control == 1)
  {
    HAL_GPIO_WritePin(CSI_SEL_GPIO_Port, CSI_SEL_Pin, SET);
    HAL_Delay(200);

    HAL_GPIO_WritePin((GPIO_TypeDef *)XSHUT_GPIO_Port, XSHUT_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin((GPIO_TypeDef *)XSHUT_GPIO_Port, XSHUT_Pin, GPIO_PIN_SET);
    HAL_Delay(50);

    vl53l9_app();
  }
  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  while (1)
//  {
//	  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//	  HAL_Delay(100);
//    /* USER CODE END WHILE */
//
//    /* USER CODE BEGIN 3 */
//  }
  /* USER CODE END 3 */
}

/**
  * @brief RIF Initialization Function
  * @param None
  * @retval None
  */
  static void SystemIsolation_Config(void)
{

/* USER CODE BEGIN RIF_Init 0 */
  __HAL_RCC_RIFSC_CLK_ENABLE();
/* USER CODE END RIF_Init 0 */

  /*RIMC configuration*/
  RIMC_MasterConfig_t RIMC_master = {0};
  RIMC_master.MasterCID = RIF_CID_1;
  RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DCMIPP, &RIMC_master);

  /* RIF-Aware IPs Config */
	/*RISUP configuration*/
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_I2C1 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_I2C2 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_UART5 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_CSI , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DCMIPP , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  /* set up PWR configuration */
  HAL_PWR_ConfigAttributes(PWR_ITEM_WKUP1,PWR_SEC_NPRIV);

  /* set up GPIO configuration */
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_14,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_15,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);

/* USER CODE BEGIN RIF_Init 1 */
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_5);
  HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_HSI, RCC_MCODIV_5);
/* USER CODE END RIF_Init 1 */
/* USER CODE BEGIN RIF_Init 2 */
//使能sram 时钟
  __HAL_RCC_AXISRAM1_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM2_MEM_CLK_ENABLE();
  __HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();
 __HAL_RCC_AXISRAM4_MEM_CLK_ENABLE();
 __HAL_RCC_AXISRAM5_MEM_CLK_ENABLE();
 __HAL_RCC_AXISRAM6_MEM_CLK_ENABLE();
/* USER CODE END RIF_Init 2 */

}

/* USER CODE BEGIN 4 */
//
static uint32_t get_risaf_max_addr(RISAF_TypeDef *risaf)
{
  uint32_t max_addr = 0U;
  if      ((risaf == RISAF1_S)  || (risaf == RISAF1_NS))  {max_addr = RISAF1_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF2_S)  || (risaf == RISAF2_NS))  {max_addr = RISAF2_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF3_S)  || (risaf == RISAF3_NS))  {max_addr = RISAF3_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF4_S)  || (risaf == RISAF4_NS))  {max_addr = RISAF4_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF5_S)  || (risaf == RISAF5_NS))  {max_addr = RISAF5_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF6_S)  || (risaf == RISAF6_NS))  {max_addr = RISAF6_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF7_S)  || (risaf == RISAF7_NS))  {max_addr = RISAF7_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF8_S)  || (risaf == RISAF8_NS))  {max_addr = RISAF8_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF9_S)  || (risaf == RISAF9_NS))  {max_addr = RISAF9_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF11_S) || (risaf == RISAF11_NS)) {max_addr = RISAF11_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF12_S) || (risaf == RISAF12_NS)) {max_addr = RISAF12_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF13_S) || (risaf == RISAF13_NS)) {max_addr = RISAF13_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF14_S) || (risaf == RISAF14_NS)) {max_addr = RISAF14_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF15_S) || (risaf == RISAF15_NS)) {max_addr = RISAF15_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF21_S) || (risaf == RISAF21_NS)) {max_addr = RISAF21_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF22_S) || (risaf == RISAF22_NS)) {max_addr = RISAF22_LIMIT_ADDRESS_SPACE_SIZE;}
  else if ((risaf == RISAF23_S) || (risaf == RISAF23_NS)) {max_addr = RISAF23_LIMIT_ADDRESS_SPACE_SIZE;}
  return max_addr;
}

static void set_risaf_default(RISAF_TypeDef *risaf)
{
  RISAF_BaseRegionConfig_t risaf_conf;  
  risaf_conf.StartAddress = 0x0;
  risaf_conf.EndAddress   = get_risaf_max_addr(risaf); /* as the default config */
  risaf_conf.Filtering    = RISAF_FILTER_ENABLE; // Base region enable (otherwise access control is secure, privileged, trusted domain CID = 1)
  risaf_conf.PrivWhitelist  = RIF_CID_NONE; // apps running in all compartments can access to region in priv/unpriv mode
  risaf_conf.ReadWhitelist  = RIF_CID_MASK; // apps running in all compartments can R in this region
  risaf_conf.WriteWhitelist = RIF_CID_MASK; // apps running in all compartments can W in this region
  // Configure 2 regions with this config, fully overlapping, one for secure one for non secure accesses:
  risaf_conf.Secure = RIF_ATTRIBUTE_SEC;    // Only secure requests can access this region
  HAL_RIF_RISAF_ConfigBaseRegion(risaf, 0, &risaf_conf);
  risaf_conf.Secure = RIF_ATTRIBUTE_NSEC;    // Only non-secure requests can access this region
  HAL_RIF_RISAF_ConfigBaseRegion(risaf, 1, &risaf_conf);
}

void RISAF_Config(void)
{
  /*
  *  Note: before to set a risaf for a given IP, the IP
  *        should be clocked.
  */
  set_risaf_default(RISAF1_S);
  set_risaf_default(RISAF2_S);          /* SRAM1_AXI */
  set_risaf_default(RISAF3_S);          /* SRAM2_AXI */
  set_risaf_default(RISAF4_S); 
  set_risaf_default(RISAF5_S); 
  set_risaf_default(RISAF6_S);          /* SRAM3,4,5,6_AXI */
  set_risaf_default(RISAF11_S);         /* XSPI1 */
  set_risaf_default(RISAF12_S);         /* XSPI2 */
}

//使能SRAM总线访问
void system_init_post(void)
{  
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_CRC_CLK_ENABLE();
   
  /* Enable NPU RAMs (4x448KB) + CACHEAXI */
  RCC->MEMENR |= RCC_MEMENR_AXISRAM3EN | RCC_MEMENR_AXISRAM4EN | RCC_MEMENR_AXISRAM5EN | RCC_MEMENR_AXISRAM6EN;
//  RCC->MEMENR |= RCC_MEMENR_CACHEAXIRAMEN; // RCC_MEMENR_NPUCACHERAMEN;
  RAMCFG_SRAM2_AXI->CR &= ~RAMCFG_CR_SRAMSD;
  RAMCFG_SRAM3_AXI->CR &= ~RAMCFG_CR_SRAMSD;
  RAMCFG_SRAM4_AXI->CR &= ~RAMCFG_CR_SRAMSD;
  RAMCFG_SRAM5_AXI->CR &= ~RAMCFG_CR_SRAMSD;
  RAMCFG_SRAM6_AXI->CR &= ~RAMCFG_CR_SRAMSD;
    
  /* Allow caches to be activated. Default value is 1, but the current boot sets it to 0 */
  MEMSYSCTL->MSCR |= MEMSYSCTL_MSCR_DCACTIVE_Msk | MEMSYSCTL_MSCR_ICACTIVE_Msk;
}

//帧完成中断处理
void HAL_DCMIPP_PIPE_FrameEventCallback(DCMIPP_HandleTypeDef *hdcmipp, uint32_t Pipe)
{
    if (Pipe == DCMIPP_PIPE0) {
        platform_evt |= PLATFORM_CAM_PIPE_FRAME_EVT;
    }
    if (Pipe == DCMIPP_PIPE1) {
      NbMainFrames++;
    }
	  
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the System Power Supply
  */
  if (HAL_PWREx_ConfigSupply(PWR_EXTERNAL_SOURCE_SUPPLY) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0) != HAL_OK)
  {
    Error_Handler();
  }

  /* Enable HSI */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL1.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Get current CPU/System buses clocks configuration and if necessary switch
 to intermediate HSI clock to ensure target clock can be set
  */
  HAL_RCC_GetClockConfig(&RCC_ClkInitStruct);
  if ((RCC_ClkInitStruct.CPUCLKSource == RCC_CPUCLKSOURCE_IC1) ||
     (RCC_ClkInitStruct.SYSCLKSource == RCC_SYSCLKSOURCE_IC2_IC6_IC11))
  {
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK);
    RCC_ClkInitStruct.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
    {
      /* Initialization Error */
      Error_Handler();
    }
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
  RCC_OscInitStruct.PLL1.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL1.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL1.PLLM = 2;
  RCC_OscInitStruct.PLL1.PLLN = 75;
  RCC_OscInitStruct.PLL1.PLLFractional = 0;
  RCC_OscInitStruct.PLL1.PLLP1 = 1;
  RCC_OscInitStruct.PLL1.PLLP2 = 4;
  RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL2.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL2.PLLM = 8;
  RCC_OscInitStruct.PLL2.PLLN = 100;
  RCC_OscInitStruct.PLL2.PLLFractional = 0;
  RCC_OscInitStruct.PLL2.PLLP1 = 1;
  RCC_OscInitStruct.PLL2.PLLP2 = 1;
  RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL3.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL3.PLLM = 8;
  RCC_OscInitStruct.PLL3.PLLN = 225;
  RCC_OscInitStruct.PLL3.PLLFractional = 0;
  RCC_OscInitStruct.PLL3.PLLP1 = 1;
  RCC_OscInitStruct.PLL3.PLLP2 = 2;
  RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_CPUCLK|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2|RCC_CLOCKTYPE_PCLK5
                              |RCC_CLOCKTYPE_PCLK4;
  RCC_ClkInitStruct.CPUCLKSource = RCC_CPUCLKSOURCE_IC1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_IC2_IC6_IC11;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;
  RCC_ClkInitStruct.APB5CLKDivider = RCC_APB5_DIV1;
  RCC_ClkInitStruct.IC1Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  RCC_ClkInitStruct.IC1Selection.ClockDivider = 1;
  RCC_ClkInitStruct.IC2Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
  RCC_ClkInitStruct.IC2Selection.ClockDivider = 2;
  RCC_ClkInitStruct.IC6Selection.ClockSelection = RCC_ICCLKSOURCE_PLL2;
  RCC_ClkInitStruct.IC6Selection.ClockDivider = 1;
  RCC_ClkInitStruct.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL3;
  RCC_ClkInitStruct.IC11Selection.ClockDivider = 3;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_5);
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
