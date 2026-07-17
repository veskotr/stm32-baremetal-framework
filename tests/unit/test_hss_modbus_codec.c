#include "hss_modbus_codec.h"
#include "unity.h"

#include <stdint.h>

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_u32_round_trip_uses_msw_lsw_order(void)
{
    uint16_t registers[2] = {0U};
    uint32_t value = 0U;

    TEST_ASSERT_EQUAL_INT(HSS_OK, hss_modbus_encode_u32(registers, 0x12345678U));
    TEST_ASSERT_EQUAL_UINT16(0x1234U, registers[0]);
    TEST_ASSERT_EQUAL_UINT16(0x5678U, registers[1]);
    TEST_ASSERT_EQUAL_INT(HSS_OK, hss_modbus_decode_u32(registers, &value));
    TEST_ASSERT_TRUE(value == 0x12345678U);
}

static void test_signed_round_trip_preserves_negative_values(void)
{
    uint16_t registers[2] = {0U};
    int32_t value = 0;

    TEST_ASSERT_EQUAL_INT(HSS_OK, hss_modbus_encode_s32(registers, -1234567));
    TEST_ASSERT_EQUAL_INT(HSS_OK, hss_modbus_decode_s32(registers, &value));
    TEST_ASSERT_TRUE(value == -1234567);
}

static void test_float_round_trip_honors_word_order(void)
{
    uint16_t registers[2] = {0U};
    float value = 0.0f;

    TEST_ASSERT_EQUAL_INT(HSS_OK,
                          hss_modbus_encode_float32(registers,
                                                     12.5f,
                                                     HSS_MODBUS_WORD_ORDER_LSW_MSW));
    TEST_ASSERT_EQUAL_INT(HSS_OK,
                          hss_modbus_decode_float32(registers,
                                                     HSS_MODBUS_WORD_ORDER_LSW_MSW,
                                                     &value));
    TEST_ASSERT_TRUE(value == 12.5f);
}

static void test_invalid_arguments_are_rejected(void)
{
    uint16_t registers[2] = {0U};

    TEST_ASSERT_EQUAL_INT(HSS_INVALID_ARGUMENT, hss_modbus_encode_u32(NULL, 1U));
    TEST_ASSERT_EQUAL_INT(HSS_INVALID_ARGUMENT, hss_modbus_decode_u32(registers, NULL));
    TEST_ASSERT_EQUAL_INT(HSS_INVALID_ARGUMENT,
                          hss_modbus_encode_float32(registers,
                                                     1.0f,
                                                     (hss_modbus_word_order_t)2));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_u32_round_trip_uses_msw_lsw_order);
    RUN_TEST(test_signed_round_trip_preserves_negative_values);
    RUN_TEST(test_float_round_trip_honors_word_order);
    RUN_TEST(test_invalid_arguments_are_rejected);
    return UNITY_END();
}
