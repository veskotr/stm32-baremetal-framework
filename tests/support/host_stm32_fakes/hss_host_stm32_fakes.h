#pragma once

#include <stddef.h>
#include <stdint.h>

#include "main.h"

typedef enum
{
    HSS_FAKE_SPI_OP_NONE = 0,
    HSS_FAKE_SPI_OP_TRANSMIT,
    HSS_FAKE_SPI_OP_RECEIVE,
    HSS_FAKE_SPI_OP_TRANSFER,
} hss_fake_spi_operation_t;

typedef struct
{
    hss_fake_spi_operation_t operation;
    SPI_HandleTypeDef *spi;
    uint8_t tx_data[256];
    uint8_t rx_seed[256];
    uint16_t length;
    uint32_t timeout_ms;
} hss_fake_spi_state_t;

typedef struct
{
    IRQn_Type irqn;
    uint32_t priority;
    uint32_t subpriority;
    uint8_t enabled[128];
} hss_fake_nvic_state_t;

typedef enum
{
    HSS_FAKE_WATCHDOG_KIND_NONE = 0,
    HSS_FAKE_WATCHDOG_KIND_IWDG,
    HSS_FAKE_WATCHDOG_KIND_WWDG,
} hss_fake_watchdog_kind_t;

typedef enum
{
    HSS_FAKE_WATCHDOG_OP_NONE = 0,
    HSS_FAKE_WATCHDOG_OP_INIT,
    HSS_FAKE_WATCHDOG_OP_REFRESH,
} hss_fake_watchdog_operation_t;

typedef struct
{
    hss_fake_watchdog_kind_t kind;
    hss_fake_watchdog_operation_t operation;
    void *watchdog;
} hss_fake_watchdog_state_t;

void hss_host_fakes_reset(void);
void hss_fake_spi_set_next_status(HAL_StatusTypeDef status);
void hss_fake_spi_set_receive_data(const uint8_t *data, uint16_t length);
const hss_fake_spi_state_t *hss_fake_spi_state(void);
const hss_fake_nvic_state_t *hss_fake_nvic_state(void);
void hss_fake_watchdog_set_next_status(HAL_StatusTypeDef status);
const hss_fake_watchdog_state_t *hss_fake_watchdog_state(void);
GPIO_PinState hss_fake_gpio_output_state(GPIO_TypeDef *port, uint16_t pin);
