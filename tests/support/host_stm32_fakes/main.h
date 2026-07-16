#pragma once

#include <stdint.h>

#define HAL_IWDG_MODULE_ENABLED 1
#define HAL_WWDG_MODULE_ENABLED 1
#define HAL_SPI_MODULE_ENABLED 1

#define SPI_POLARITY_LOW 0U
#define SPI_POLARITY_HIGH 1U
#define SPI_PHASE_1EDGE 0U
#define SPI_PHASE_2EDGE 1U
#define SPI_BAUDRATEPRESCALER_16 16U
#define SPI_NSS_SOFT 1U
#define SPI_FIRSTBIT_MSB 0U

#define GPIO_PIN_0 ((uint16_t)0x0001U)
#define GPIO_PIN_1 ((uint16_t)0x0002U)
#define GPIO_PIN_2 ((uint16_t)0x0004U)
#define GPIO_PIN_3 ((uint16_t)0x0008U)
#define GPIO_PIN_4 ((uint16_t)0x0010U)
#define GPIO_PIN_5 ((uint16_t)0x0020U)
#define GPIO_PIN_6 ((uint16_t)0x0040U)
#define GPIO_PIN_7 ((uint16_t)0x0080U)
#define GPIO_PIN_8 ((uint16_t)0x0100U)
#define GPIO_PIN_9 ((uint16_t)0x0200U)
#define GPIO_PIN_10 ((uint16_t)0x0400U)
#define GPIO_PIN_11 ((uint16_t)0x0800U)
#define GPIO_PIN_12 ((uint16_t)0x1000U)
#define GPIO_PIN_13 ((uint16_t)0x2000U)
#define GPIO_PIN_14 ((uint16_t)0x4000U)
#define GPIO_PIN_15 ((uint16_t)0x8000U)

typedef enum
{
    HAL_OK = 0,
    HAL_ERROR = 1,
    HAL_BUSY = 2,
    HAL_TIMEOUT = 3,
} HAL_StatusTypeDef;

typedef enum
{
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET = 1,
} GPIO_PinState;

typedef struct
{
    uint32_t odr;
    uint32_t idr;
} GPIO_TypeDef;

typedef enum
{
    EXTI0_IRQn = 6,
    EXTI1_IRQn = 7,
    EXTI2_IRQn = 8,
    EXTI3_IRQn = 9,
    EXTI4_IRQn = 10,
    EXTI9_5_IRQn = 23,
    EXTI15_10_IRQn = 40,
    EXTI0_1_IRQn = 5,
    EXTI2_3_IRQn = 6,
    EXTI4_15_IRQn = 7,
} IRQn_Type;

typedef struct
{
    uint32_t CLKPolarity;
    uint32_t CLKPhase;
    uint32_t BaudRatePrescaler;
    uint32_t NSS;
    uint32_t FirstBit;
} SPI_InitTypeDef;

typedef struct
{
    int instance;
    SPI_InitTypeDef Init;
} SPI_HandleTypeDef;

typedef struct
{
    int instance;
} IWDG_HandleTypeDef;

typedef struct
{
    int instance;
} WWDG_HandleTypeDef;

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);
void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);
void HAL_NVIC_DisableIRQ(IRQn_Type IRQn);

HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi,
                                   uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi,
                                  uint8_t *pData,
                                  uint16_t Size,
                                  uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi,
                                          uint8_t *pTxData,
                                  uint8_t *pRxData,
                                  uint16_t Size,
                                  uint32_t Timeout);

HAL_StatusTypeDef HAL_IWDG_Init(IWDG_HandleTypeDef *hiwdg);
HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg);
HAL_StatusTypeDef HAL_WWDG_Init(WWDG_HandleTypeDef *hwwdg);
HAL_StatusTypeDef HAL_WWDG_Refresh(WWDG_HandleTypeDef *hwwdg);
