#include "hss_host_stm32_fakes.h"

#include <string.h>

static HAL_StatusTypeDef fake_spi_next_status = HAL_OK;
static hss_fake_spi_state_t fake_spi_state;
static hss_fake_nvic_state_t fake_nvic_state;

void hss_host_fakes_reset(void)
{
    fake_spi_next_status = HAL_OK;
    memset(&fake_spi_state, 0, sizeof(fake_spi_state));
    memset(&fake_nvic_state, 0, sizeof(fake_nvic_state));
}

void hss_fake_spi_set_next_status(HAL_StatusTypeDef status)
{
    fake_spi_next_status = status;
}

void hss_fake_spi_set_receive_data(const uint8_t *data, uint16_t length)
{
    if (data == NULL)
    {
        return;
    }

    if (length > sizeof(fake_spi_state.rx_seed))
    {
        length = (uint16_t)sizeof(fake_spi_state.rx_seed);
    }
    memcpy(fake_spi_state.rx_seed, data, length);
}

const hss_fake_spi_state_t *hss_fake_spi_state(void)
{
    return &fake_spi_state;
}

const hss_fake_nvic_state_t *hss_fake_nvic_state(void)
{
    return &fake_nvic_state;
}

GPIO_PinState hss_fake_gpio_output_state(GPIO_TypeDef *port, uint16_t pin)
{
    if (port == NULL || pin == 0U)
    {
        return GPIO_PIN_RESET;
    }

    return (port->odr & pin) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if (GPIOx == NULL)
    {
        return GPIO_PIN_RESET;
    }

    return (GPIOx->idr & GPIO_Pin) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    if (GPIOx == NULL)
    {
        return;
    }

    if (PinState == GPIO_PIN_SET)
    {
        GPIOx->odr |= GPIO_Pin;
    }
    else
    {
        GPIOx->odr &= (uint32_t)~GPIO_Pin;
    }
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if (GPIOx == NULL)
    {
        return;
    }

    GPIOx->odr ^= GPIO_Pin;
}

void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{
    fake_nvic_state.irqn = IRQn;
    fake_nvic_state.priority = PreemptPriority;
    fake_nvic_state.subpriority = SubPriority;
}

void HAL_NVIC_EnableIRQ(IRQn_Type IRQn)
{
    if ((int)IRQn >= 0 && (int)IRQn < (int)sizeof(fake_nvic_state.enabled))
    {
        fake_nvic_state.enabled[(int)IRQn] = 1U;
    }
}

void HAL_NVIC_DisableIRQ(IRQn_Type IRQn)
{
    if ((int)IRQn >= 0 && (int)IRQn < (int)sizeof(fake_nvic_state.enabled))
    {
        fake_nvic_state.enabled[(int)IRQn] = 0U;
    }
}

HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef *hspi)
{
    fake_spi_state.spi = hspi;
    return fake_spi_next_status;
}

HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi,
                                   uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout)
{
    fake_spi_state.operation = HSS_FAKE_SPI_OP_TRANSMIT;
    fake_spi_state.spi = hspi;
    fake_spi_state.length = Size;
    fake_spi_state.timeout_ms = Timeout;
    if (pData != NULL && Size <= sizeof(fake_spi_state.tx_data))
    {
        memcpy(fake_spi_state.tx_data, pData, Size);
    }
    return fake_spi_next_status;
}

HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi,
                                  uint8_t *pData,
                                  uint16_t Size,
                                  uint32_t Timeout)
{
    fake_spi_state.operation = HSS_FAKE_SPI_OP_RECEIVE;
    fake_spi_state.spi = hspi;
    fake_spi_state.length = Size;
    fake_spi_state.timeout_ms = Timeout;
    if (pData != NULL && Size <= sizeof(fake_spi_state.rx_seed))
    {
        memcpy(pData, fake_spi_state.rx_seed, Size);
    }
    return fake_spi_next_status;
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi,
                                          uint8_t *pTxData,
                                          uint8_t *pRxData,
                                          uint16_t Size,
                                          uint32_t Timeout)
{
    fake_spi_state.operation = HSS_FAKE_SPI_OP_TRANSFER;
    fake_spi_state.spi = hspi;
    fake_spi_state.length = Size;
    fake_spi_state.timeout_ms = Timeout;
    if (pTxData != NULL && Size <= sizeof(fake_spi_state.tx_data))
    {
        memcpy(fake_spi_state.tx_data, pTxData, Size);
    }
    if (pRxData != NULL && Size <= sizeof(fake_spi_state.rx_seed))
    {
        memcpy(pRxData, fake_spi_state.rx_seed, Size);
    }
    return fake_spi_next_status;
}
