#include "hss_modbus.h"

#include <stdbool.h>
#include <stddef.h>

#include "hss_modbus_timer.h"
#include "hss_modbus_uart.h"
#include "mb.h"

static hss_modbus_config_t g_modbus_config;
static bool g_modbus_initialized;
#if HSS_ENABLE_MODBUS_DEBUG
volatile uint32_t hss_modbus_debug_input_cb_count;
volatile uint32_t hss_modbus_debug_holding_cb_count;
volatile uint32_t hss_modbus_debug_holding_write_count;
#endif

static hss_result_t hss_modbus_result_from_mb(eMBErrorCode error)
{
    switch (error)
    {
    case MB_ENOERR:
        return HSS_OK;
    case MB_EINVAL:
        return HSS_INVALID_ARGUMENT;
    case MB_EPORTERR:
        return HSS_NOT_SUPPORTED;
    case MB_ENORES:
        return HSS_BUSY;
    case MB_EIO:
        return HSS_ERROR;
    case MB_EILLSTATE:
        return HSS_INVALID_STATE;
    case MB_ETIMEDOUT:
        return HSS_TIMEOUT;
    default:
        return HSS_ERROR;
    }
}

static eMBMode hss_modbus_to_mb_mode(hss_modbus_mode_t mode)
{
    switch (mode)
    {
    case HSS_MODBUS_MODE_ASCII:
        return MB_ASCII;
    case HSS_MODBUS_MODE_RTU:
    default:
        return MB_RTU;
    }
}

static eMBParity hss_modbus_to_mb_parity(hss_modbus_parity_t parity)
{
    switch (parity)
    {
    case HSS_MODBUS_PARITY_ODD:
        return MB_PAR_ODD;
    case HSS_MODBUS_PARITY_EVEN:
        return MB_PAR_EVEN;
    case HSS_MODBUS_PARITY_NONE:
    default:
        return MB_PAR_NONE;
    }
}

static bool hss_modbus_register_bank_contains(const hss_modbus_register_bank_t *bank,
                                              uint16_t address,
                                              uint16_t count,
                                              uint16_t *offset)
{
    if (bank == NULL || bank->values == NULL || bank->count == 0U || count == 0U)
    {
        return false;
    }

    const uint32_t bank_start = bank->start_address;
    const uint32_t bank_end = bank_start + bank->count;
    const uint32_t request_start = address;
    const uint32_t request_end = request_start + count;

    if (request_start < bank_start || request_end > bank_end)
    {
        return false;
    }

    if (offset != NULL)
    {
        *offset = (uint16_t)(request_start - bank_start);
    }

    return true;
}

static bool hss_modbus_callback_address_to_api_address(USHORT modbus_address, uint16_t *api_address)
{
    if (api_address == NULL || modbus_address == 0U)
    {
        return false;
    }

    *api_address = (uint16_t)(modbus_address - 1U);
    return true;
}

static hss_result_t hss_modbus_get_register(const hss_modbus_register_bank_t *bank,
                                            uint16_t address,
                                            uint16_t *value)
{
    uint16_t offset = 0U;

    if (value == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    if (!g_modbus_initialized)
    {
        return HSS_INVALID_STATE;
    }

    if (!hss_modbus_register_bank_contains(bank, address, 1U, &offset))
    {
        return HSS_NOT_SUPPORTED;
    }

    *value = bank->values[offset];
    return HSS_OK;
}

static hss_result_t hss_modbus_set_register(const hss_modbus_register_bank_t *bank,
                                            uint16_t address,
                                            uint16_t value)
{
    uint16_t offset = 0U;

    if (!g_modbus_initialized)
    {
        return HSS_INVALID_STATE;
    }

    if (!hss_modbus_register_bank_contains(bank, address, 1U, &offset))
    {
        return HSS_NOT_SUPPORTED;
    }

    bank->values[offset] = value;
    return HSS_OK;
}

hss_modbus_config_t hss_modbus_default_config(uint8_t slave_id, uint32_t baudrate)
{
    hss_modbus_config_t config = {
        .slave_id = slave_id,
        .baudrate = baudrate,
        .mode = HSS_MODBUS_MODE_RTU,
        .parity = HSS_MODBUS_PARITY_NONE,
        .holding_registers = {0},
        .input_registers = {0},
    };

    return config;
}

hss_result_t hss_modbus_init(const hss_modbus_config_t *config)
{
    if (config == NULL || config->slave_id == 0U || config->slave_id > 247U || config->baudrate == 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

    if (!hss_modbus_uart_is_available() || !hss_modbus_timer_is_available())
    {
        return HSS_NOT_SUPPORTED;
    }

    g_modbus_config = *config;
    g_modbus_initialized = false;

    const eMBErrorCode error = eMBInit(hss_modbus_to_mb_mode(config->mode),
                                       config->slave_id,
                                       0U,
                                       config->baudrate,
                                       hss_modbus_to_mb_parity(config->parity));
    const hss_result_t result = hss_modbus_result_from_mb(error);
    if (result != HSS_OK)
    {
        return result;
    }

    g_modbus_initialized = true;
    return HSS_OK;
}

hss_result_t hss_modbus_enable(void)
{
    if (!g_modbus_initialized)
    {
        return HSS_INVALID_STATE;
    }

    return hss_modbus_result_from_mb(eMBEnable());
}

hss_result_t hss_modbus_disable(void)
{
    if (!g_modbus_initialized)
    {
        return HSS_INVALID_STATE;
    }

    return hss_modbus_result_from_mb(eMBDisable());
}

hss_result_t hss_modbus_poll(void)
{
    if (!g_modbus_initialized)
    {
        return HSS_INVALID_STATE;
    }

    return hss_modbus_result_from_mb(eMBPoll());
}

hss_result_t hss_modbus_get_holding_register(uint16_t address, uint16_t *value)
{
    return hss_modbus_get_register(&g_modbus_config.holding_registers, address, value);
}

hss_result_t hss_modbus_set_holding_register(uint16_t address, uint16_t value)
{
    return hss_modbus_set_register(&g_modbus_config.holding_registers, address, value);
}

hss_result_t hss_modbus_get_input_register(uint16_t address, uint16_t *value)
{
    return hss_modbus_get_register(&g_modbus_config.input_registers, address, value);
}

hss_result_t hss_modbus_set_input_register(uint16_t address, uint16_t value)
{
    return hss_modbus_set_register(&g_modbus_config.input_registers, address, value);
}

eMBErrorCode eMBRegInputCB(UCHAR *buffer, USHORT address, USHORT count)
{
    uint16_t api_address = 0U;
    uint16_t offset = 0U;

#if HSS_ENABLE_MODBUS_DEBUG
    hss_modbus_debug_input_cb_count++;
#endif

    if (buffer == NULL || !hss_modbus_callback_address_to_api_address(address, &api_address))
    {
        return MB_EINVAL;
    }

    if (!hss_modbus_register_bank_contains(&g_modbus_config.input_registers, api_address, count, &offset))
    {
        return MB_ENOREG;
    }

    for (USHORT i = 0U; i < count; i++)
    {
        const uint16_t value = g_modbus_config.input_registers.values[offset + i];
        buffer[2U * i] = (UCHAR)(value >> 8U);
        buffer[(2U * i) + 1U] = (UCHAR)(value & 0xFFU);
    }

    return MB_ENOERR;
}

eMBErrorCode eMBRegHoldingCB(UCHAR *buffer, USHORT address, USHORT count, eMBRegisterMode mode)
{
    uint16_t api_address = 0U;
    uint16_t offset = 0U;

#if HSS_ENABLE_MODBUS_DEBUG
    hss_modbus_debug_holding_cb_count++;
#endif

    if (buffer == NULL || !hss_modbus_callback_address_to_api_address(address, &api_address))
    {
        return MB_EINVAL;
    }

    if (!hss_modbus_register_bank_contains(&g_modbus_config.holding_registers, api_address, count, &offset))
    {
        return MB_ENOREG;
    }

    for (USHORT i = 0U; i < count; i++)
    {
        if (mode == MB_REG_READ)
        {
            const uint16_t value = g_modbus_config.holding_registers.values[offset + i];
            buffer[2U * i] = (UCHAR)(value >> 8U);
            buffer[(2U * i) + 1U] = (UCHAR)(value & 0xFFU);
        }
        else
        {
            g_modbus_config.holding_registers.values[offset + i] =
                (uint16_t)(((uint16_t)buffer[2U * i] << 8U) | buffer[(2U * i) + 1U]);
#if HSS_ENABLE_MODBUS_DEBUG
            hss_modbus_debug_holding_write_count++;
#endif
        }
    }

    return MB_ENOERR;
}

eMBErrorCode eMBRegCoilsCB(UCHAR *buffer, USHORT address, USHORT count, eMBRegisterMode mode)
{
    (void)buffer;
    (void)address;
    (void)count;
    (void)mode;
    return MB_ENOREG;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR *buffer, USHORT address, USHORT count)
{
    (void)buffer;
    (void)address;
    (void)count;
    return MB_ENOREG;
}
