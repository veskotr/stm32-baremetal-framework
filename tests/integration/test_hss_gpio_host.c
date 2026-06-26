#include "unity.h"

#include "hss_gpio.h"
#include "hss_host_stm32_fakes.h"

static uint16_t callback_pin;
static int callback_count;
static void *callback_context;

static void record_interrupt(uint16_t pin, void *context)
{
    callback_pin = pin;
    callback_context = context;
    callback_count++;
}

static void reset_callback_state(void)
{
    callback_pin = 0U;
    callback_count = 0;
    callback_context = NULL;
}

static void gpio_rejects_invalid_arguments(void)
{
    GPIO_TypeDef port = {0};
    GPIO_PinState state = GPIO_PIN_RESET;

    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_read(NULL, GPIO_PIN_0, &state));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_read(&port, 0U, &state));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_read(&port, GPIO_PIN_0, NULL));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_write(NULL, GPIO_PIN_0, GPIO_PIN_SET));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_write(&port, 0U, GPIO_PIN_SET));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_toggle(NULL, GPIO_PIN_0));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_toggle(&port, 0U));
}

static void gpio_fake_records_read_write_and_toggle_state(void)
{
    GPIO_TypeDef port = {0};
    GPIO_PinState state = GPIO_PIN_RESET;

    port.idr = GPIO_PIN_1;
    TEST_ASSERT_EQUAL(HSS_OK, hss_gpio_read(&port, GPIO_PIN_1, &state));
    TEST_ASSERT_EQUAL(GPIO_PIN_SET, state);

    TEST_ASSERT_EQUAL(HSS_OK, hss_gpio_write(&port, GPIO_PIN_2, GPIO_PIN_SET));
    TEST_ASSERT_EQUAL(GPIO_PIN_SET, hss_fake_gpio_output_state(&port, GPIO_PIN_2));

    TEST_ASSERT_EQUAL(HSS_OK, hss_gpio_toggle(&port, GPIO_PIN_2));
    TEST_ASSERT_EQUAL(GPIO_PIN_RESET, hss_fake_gpio_output_state(&port, GPIO_PIN_2));
}

static void gpio_interrupt_dispatch_invokes_registered_callback(void)
{
    int context = 42;

    reset_callback_state();
    TEST_ASSERT_EQUAL(HSS_OK, hss_gpio_register_interrupt(GPIO_PIN_3, record_interrupt, &context));
    TEST_ASSERT_EQUAL(HSS_OK, hss_gpio_dispatch_interrupt(GPIO_PIN_3));
    TEST_ASSERT_EQUAL_UINT16(GPIO_PIN_3, callback_pin);
    TEST_ASSERT_EQUAL_INT(1, callback_count);
    TEST_ASSERT_TRUE(callback_context == &context);

    TEST_ASSERT_EQUAL(HSS_OK, hss_gpio_unregister_interrupt(GPIO_PIN_3));
    TEST_ASSERT_EQUAL(HSS_NOT_READY, hss_gpio_dispatch_interrupt(GPIO_PIN_3));
}

static void gpio_interrupt_rejects_invalid_pins(void)
{
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_register_interrupt(0U, record_interrupt, NULL));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_register_interrupt((uint16_t)(GPIO_PIN_0 | GPIO_PIN_1),
                                                                        record_interrupt,
                                                                        NULL));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_register_interrupt(GPIO_PIN_0, NULL, NULL));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_unregister_interrupt(0U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_gpio_dispatch_interrupt(0U));
}

int main(void)
{
    hss_host_fakes_reset();
    UNITY_BEGIN();
    RUN_TEST(gpio_rejects_invalid_arguments);
    RUN_TEST(gpio_fake_records_read_write_and_toggle_state);
    RUN_TEST(gpio_interrupt_dispatch_invokes_registered_callback);
    RUN_TEST(gpio_interrupt_rejects_invalid_pins);
    return UNITY_END();
}
