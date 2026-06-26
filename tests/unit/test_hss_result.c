#include "unity.h"

#include "hss_result.h"

static void result_helpers_classify_ok_and_errors(void)
{
    TEST_ASSERT_TRUE(hss_result_is_ok(HSS_OK));
    TEST_ASSERT_FALSE(hss_result_is_error(HSS_OK));
    TEST_ASSERT_FALSE(hss_result_is_ok(HSS_BUSY));
    TEST_ASSERT_TRUE(hss_result_is_error(HSS_BUSY));
}

static void result_to_string_returns_stable_names(void)
{
    TEST_ASSERT_EQUAL_STRING("ok", hss_result_to_string(HSS_OK));
    TEST_ASSERT_EQUAL_STRING("error", hss_result_to_string(HSS_ERROR));
    TEST_ASSERT_EQUAL_STRING("invalid_argument", hss_result_to_string(HSS_INVALID_ARGUMENT));
    TEST_ASSERT_EQUAL_STRING("timeout", hss_result_to_string(HSS_TIMEOUT));
    TEST_ASSERT_EQUAL_STRING("busy", hss_result_to_string(HSS_BUSY));
    TEST_ASSERT_EQUAL_STRING("not_ready", hss_result_to_string(HSS_NOT_READY));
    TEST_ASSERT_EQUAL_STRING("not_supported", hss_result_to_string(HSS_NOT_SUPPORTED));
    TEST_ASSERT_EQUAL_STRING("invalid_state", hss_result_to_string(HSS_INVALID_STATE));
    TEST_ASSERT_EQUAL_STRING("unknown", hss_result_to_string((hss_result_t)99));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(result_helpers_classify_ok_and_errors);
    RUN_TEST(result_to_string_returns_stable_names);
    return UNITY_END();
}
