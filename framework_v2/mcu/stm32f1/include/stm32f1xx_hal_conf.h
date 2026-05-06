#pragma once

/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED

/* ########################## HAL Includes ################################# */
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_flash.h"

#ifdef DEBUG
#include <assert.h>
#define assert_param(expr) assert(expr)
#else
#define assert_param(expr) ((void)0U)
#endif