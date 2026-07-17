#include "hss_modbus_codec.h"

#include <stddef.h>
#include <string.h>

static hss_result_t hss_modbus_validate_word_order(hss_modbus_word_order_t word_order)
{
    return word_order == HSS_MODBUS_WORD_ORDER_MSW_LSW ||
                   word_order == HSS_MODBUS_WORD_ORDER_LSW_MSW
               ? HSS_OK
               : HSS_INVALID_ARGUMENT;
}

hss_result_t hss_modbus_encode_u32(uint16_t registers[2], uint32_t value)
{
    if (registers == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    registers[0] = (uint16_t)(value >> 16U);
    registers[1] = (uint16_t)value;
    return HSS_OK;
}

hss_result_t hss_modbus_decode_u32(const uint16_t registers[2], uint32_t *value)
{
    if (registers == NULL || value == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    *value = ((uint32_t)registers[0] << 16U) | registers[1];
    return HSS_OK;
}

hss_result_t hss_modbus_encode_s32(uint16_t registers[2], int32_t value)
{
    return hss_modbus_encode_u32(registers, (uint32_t)value);
}

hss_result_t hss_modbus_decode_s32(const uint16_t registers[2], int32_t *value)
{
    if (value == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    uint32_t raw_value = 0U;
    const hss_result_t result = hss_modbus_decode_u32(registers, &raw_value);
    if (result != HSS_OK)
    {
        return result;
    }

    *value = (int32_t)raw_value;
    return HSS_OK;
}

hss_result_t hss_modbus_encode_float32(uint16_t registers[2],
                                        float value,
                                        hss_modbus_word_order_t word_order)
{
    if (registers == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    const hss_result_t result = hss_modbus_validate_word_order(word_order);
    if (result != HSS_OK)
    {
        return result;
    }

    uint32_t raw_value = 0U;
    memcpy(&raw_value, &value, sizeof(raw_value));

    if (word_order == HSS_MODBUS_WORD_ORDER_LSW_MSW)
    {
        registers[0] = (uint16_t)raw_value;
        registers[1] = (uint16_t)(raw_value >> 16U);
    }
    else
    {
        registers[0] = (uint16_t)(raw_value >> 16U);
        registers[1] = (uint16_t)raw_value;
    }

    return HSS_OK;
}

hss_result_t hss_modbus_decode_float32(const uint16_t registers[2],
                                        hss_modbus_word_order_t word_order,
                                        float *value)
{
    if (registers == NULL || value == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    const hss_result_t result = hss_modbus_validate_word_order(word_order);
    if (result != HSS_OK)
    {
        return result;
    }

    const uint32_t raw_value = word_order == HSS_MODBUS_WORD_ORDER_LSW_MSW
                                   ? ((uint32_t)registers[1] << 16U) | registers[0]
                                   : ((uint32_t)registers[0] << 16U) | registers[1];
    memcpy(value, &raw_value, sizeof(raw_value));
    return HSS_OK;
}
