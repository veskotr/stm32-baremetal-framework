#pragma once

#include <stdint.h>

#include "hss_result.h"

typedef enum
{
    HSS_MODBUS_WORD_ORDER_MSW_LSW = 0,
    HSS_MODBUS_WORD_ORDER_LSW_MSW = 1,
} hss_modbus_word_order_t;

hss_result_t hss_modbus_encode_u32(uint16_t registers[2], uint32_t value);
hss_result_t hss_modbus_decode_u32(const uint16_t registers[2], uint32_t *value);
hss_result_t hss_modbus_encode_s32(uint16_t registers[2], int32_t value);
hss_result_t hss_modbus_decode_s32(const uint16_t registers[2], int32_t *value);
hss_result_t hss_modbus_encode_float32(uint16_t registers[2],
                                        float value,
                                        hss_modbus_word_order_t word_order);
hss_result_t hss_modbus_decode_float32(const uint16_t registers[2],
                                        hss_modbus_word_order_t word_order,
                                        float *value);
