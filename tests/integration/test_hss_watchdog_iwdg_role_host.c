#include "unity.h"

#include "hss_host_stm32_fakes.h"
#include "hss_board_watchdog.h"
#include "hss_watchdog.h"

IWDG_HandleTypeDef hiwdg = {.instance = 1};

static void watchdog_iwdg_board_role_dispatches_to_the_board_handle(void)
{
    hss_host_fakes_reset();

    TEST_ASSERT_TRUE(hss_watchdog_is_available());
    TEST_ASSERT_EQUAL(HSS_OK, hss_watchdog_init());
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_KIND_IWDG, hss_fake_watchdog_state()->kind);
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_OP_INIT, hss_fake_watchdog_state()->operation);
    TEST_ASSERT_TRUE(hss_fake_watchdog_state()->watchdog == &hiwdg);

    TEST_ASSERT_EQUAL(HSS_OK, hss_watchdog_refresh());
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_KIND_IWDG, hss_fake_watchdog_state()->kind);
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_OP_REFRESH, hss_fake_watchdog_state()->operation);
    TEST_ASSERT_TRUE(hss_fake_watchdog_state()->watchdog == &hiwdg);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(watchdog_iwdg_board_role_dispatches_to_the_board_handle);
    return UNITY_END();
}
