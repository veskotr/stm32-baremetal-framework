#include "unity.h"

#include "hss_exti.h"
#include "hss_host_stm32_fakes.h"

static uint16_t callback_pin;
static int callback_count;

static void record_interrupt(uint16_t pin, void *context)
{
    (void)context;
    callback_pin = pin;
    callback_count++;
}

static void exti_input_rejects_invalid_arguments(void)
{
    hss_exti_input_t input = {
        .pin = 0U,
        .irqn = EXTI3_IRQn,
        .priority = 1U,
        .subpriority = 0U,
    };

    TEST_ASSERT_FALSE(hss_exti_input_is_available(NULL));
    TEST_ASSERT_FALSE(hss_exti_input_is_available(&input));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_exti_input_enable(NULL));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_exti_input_enable(&input));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_exti_input_disable(NULL));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_exti_input_register_callback(NULL, record_interrupt, NULL));
}

static void exti_input_enable_and_disable_use_nvic(void)
{
    hss_exti_input_t input = {
        .pin = GPIO_PIN_3,
        .irqn = EXTI3_IRQn,
        .priority = 2U,
        .subpriority = 1U,
    };

    hss_host_fakes_reset();
    TEST_ASSERT_TRUE(hss_exti_input_is_available(&input));
    TEST_ASSERT_EQUAL(HSS_OK, hss_exti_input_enable(&input));

    const hss_fake_nvic_state_t *nvic = hss_fake_nvic_state();
    TEST_ASSERT_EQUAL(EXTI3_IRQn, nvic->irqn);
    TEST_ASSERT_EQUAL_UINT(2U, nvic->priority);
    TEST_ASSERT_EQUAL_UINT(1U, nvic->subpriority);
    TEST_ASSERT_EQUAL_UINT(1U, nvic->enabled[EXTI3_IRQn]);

    TEST_ASSERT_EQUAL(HSS_OK, hss_exti_input_disable(&input));
    TEST_ASSERT_EQUAL_UINT(0U, nvic->enabled[EXTI3_IRQn]);
}

static void exti_input_registers_gpio_callback(void)
{
    hss_exti_input_t input = {
        .pin = GPIO_PIN_4,
        .irqn = EXTI4_IRQn,
        .priority = 0U,
        .subpriority = 0U,
    };

    callback_pin = 0U;
    callback_count = 0;
    TEST_ASSERT_EQUAL(HSS_OK, hss_exti_input_register_callback(&input, record_interrupt, NULL));
    TEST_ASSERT_EQUAL(HSS_OK, hss_gpio_dispatch_interrupt(GPIO_PIN_4));
    TEST_ASSERT_EQUAL_UINT16(GPIO_PIN_4, callback_pin);
    TEST_ASSERT_EQUAL_INT(1, callback_count);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(exti_input_rejects_invalid_arguments);
    RUN_TEST(exti_input_enable_and_disable_use_nvic);
    RUN_TEST(exti_input_registers_gpio_callback);
    return UNITY_END();
}
