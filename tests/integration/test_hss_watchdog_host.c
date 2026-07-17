#include "unity.h"

#include "hss_host_stm32_fakes.h"
#include "hss_board_watchdog.h"
#include "hss_watchdog.h"

static void watchdog_rejects_invalid_arguments(void)
{
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_iwdg_init(NULL));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_iwdg_refresh(NULL));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_wwdg_init(NULL));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_wwdg_refresh(NULL));
}

static void watchdog_iwdg_fakes_record_init_and_refresh(void)
{
    IWDG_HandleTypeDef iwdg = {0};

    hss_fake_watchdog_set_next_status(HAL_TIMEOUT);
    TEST_ASSERT_EQUAL(HSS_TIMEOUT, hss_iwdg_init(&iwdg));
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_KIND_IWDG, hss_fake_watchdog_state()->kind);
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_OP_INIT, hss_fake_watchdog_state()->operation);
    TEST_ASSERT_TRUE(hss_fake_watchdog_state()->watchdog == &iwdg);

    hss_fake_watchdog_set_next_status(HAL_OK);
    TEST_ASSERT_EQUAL(HSS_OK, hss_iwdg_refresh(&iwdg));
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_KIND_IWDG, hss_fake_watchdog_state()->kind);
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_OP_REFRESH, hss_fake_watchdog_state()->operation);
    TEST_ASSERT_TRUE(hss_fake_watchdog_state()->watchdog == &iwdg);
}

static void watchdog_wwdg_fakes_record_init_and_refresh(void)
{
    WWDG_HandleTypeDef wwdg = {0};

    hss_fake_watchdog_set_next_status(HAL_BUSY);
    TEST_ASSERT_EQUAL(HSS_BUSY, hss_wwdg_init(&wwdg));
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_KIND_WWDG, hss_fake_watchdog_state()->kind);
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_OP_INIT, hss_fake_watchdog_state()->operation);
    TEST_ASSERT_TRUE(hss_fake_watchdog_state()->watchdog == &wwdg);

    hss_fake_watchdog_set_next_status(HAL_ERROR);
    TEST_ASSERT_EQUAL(HSS_ERROR, hss_wwdg_refresh(&wwdg));
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_KIND_WWDG, hss_fake_watchdog_state()->kind);
    TEST_ASSERT_EQUAL(HSS_FAKE_WATCHDOG_OP_REFRESH, hss_fake_watchdog_state()->operation);
    TEST_ASSERT_TRUE(hss_fake_watchdog_state()->watchdog == &wwdg);
}

static void watchdog_board_level_helpers_report_absence_without_role(void)
{
    TEST_ASSERT_FALSE(hss_watchdog_is_available());
    TEST_ASSERT_EQUAL(HSS_NOT_SUPPORTED, hss_watchdog_init());
    TEST_ASSERT_EQUAL(HSS_NOT_SUPPORTED, hss_watchdog_refresh());
}

int main(void)
{
    hss_host_fakes_reset();
    UNITY_BEGIN();
    RUN_TEST(watchdog_rejects_invalid_arguments);
    RUN_TEST(watchdog_iwdg_fakes_record_init_and_refresh);
    RUN_TEST(watchdog_wwdg_fakes_record_init_and_refresh);
    RUN_TEST(watchdog_board_level_helpers_report_absence_without_role);
    return UNITY_END();
}
