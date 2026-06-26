#include "unity.h"

#include "hss_host_stm32_fakes.h"
#include "hss_spi.h"

static void spi_rejects_invalid_arguments(void)
{
    SPI_HandleTypeDef spi = {0};
    uint8_t tx[1] = {0x12U};
    uint8_t rx[1] = {0};

    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_write(NULL, tx, 1U, 10U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_write(&spi, NULL, 1U, 10U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_write(&spi, tx, 0U, 10U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_read(NULL, rx, 1U, 10U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_read(&spi, NULL, 1U, 10U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_read(&spi, rx, 0U, 10U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_transfer(NULL, tx, rx, 1U, 10U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_transfer(&spi, NULL, rx, 1U, 10U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_transfer(&spi, tx, NULL, 1U, 10U));
    TEST_ASSERT_EQUAL(HSS_INVALID_ARGUMENT, hss_spi_transfer(&spi, tx, rx, 0U, 10U));
}

static void spi_maps_hal_statuses_to_hss_results(void)
{
    SPI_HandleTypeDef spi = {0};
    uint8_t tx[1] = {0x34U};

    hss_fake_spi_set_next_status(HAL_OK);
    TEST_ASSERT_EQUAL(HSS_OK, hss_spi_write(&spi, tx, 1U, 10U));
    hss_fake_spi_set_next_status(HAL_BUSY);
    TEST_ASSERT_EQUAL(HSS_BUSY, hss_spi_write(&spi, tx, 1U, 10U));
    hss_fake_spi_set_next_status(HAL_TIMEOUT);
    TEST_ASSERT_EQUAL(HSS_TIMEOUT, hss_spi_write(&spi, tx, 1U, 10U));
    hss_fake_spi_set_next_status(HAL_ERROR);
    TEST_ASSERT_EQUAL(HSS_ERROR, hss_spi_write(&spi, tx, 1U, 10U));
}

static void spi_fake_records_transfer_data_and_timeout(void)
{
    SPI_HandleTypeDef spi = {.instance = 7};
    uint8_t tx[3] = {0xA1U, 0xB2U, 0xC3U};
    uint8_t seed[3] = {0x01U, 0x02U, 0x03U};
    uint8_t rx[3] = {0};

    hss_host_fakes_reset();
    hss_fake_spi_set_receive_data(seed, sizeof(seed));
    TEST_ASSERT_EQUAL(HSS_OK, hss_spi_transfer(&spi, tx, rx, sizeof(tx), 25U));

    const hss_fake_spi_state_t *state = hss_fake_spi_state();
    TEST_ASSERT_EQUAL(HSS_FAKE_SPI_OP_TRANSFER, state->operation);
    TEST_ASSERT_TRUE(state->spi == &spi);
    TEST_ASSERT_EQUAL_UINT16(sizeof(tx), state->length);
    TEST_ASSERT_EQUAL_UINT(25U, state->timeout_ms);
    TEST_ASSERT_EQUAL_MEMORY(tx, state->tx_data, sizeof(tx));
    TEST_ASSERT_EQUAL_MEMORY(seed, rx, sizeof(seed));
}

static void spi_device_controls_active_low_chip_select_around_transfer(void)
{
    SPI_HandleTypeDef spi = {0};
    GPIO_TypeDef cs_port = {.odr = GPIO_PIN_4};
    uint8_t tx[1] = {0x55U};
    uint8_t rx[1] = {0};
    hss_spi_device_t device = {
        .spi = &spi,
        .cs_port = &cs_port,
        .cs_pin = GPIO_PIN_4,
        .cs_active_high = false,
    };

    TEST_ASSERT_EQUAL(HSS_OK, hss_spi_device_select(&device));
    TEST_ASSERT_EQUAL(GPIO_PIN_RESET, hss_fake_gpio_output_state(&cs_port, GPIO_PIN_4));
    TEST_ASSERT_EQUAL(HSS_OK, hss_spi_device_deselect(&device));
    TEST_ASSERT_EQUAL(GPIO_PIN_SET, hss_fake_gpio_output_state(&cs_port, GPIO_PIN_4));

    hss_host_fakes_reset();
    TEST_ASSERT_EQUAL(HSS_OK, hss_spi_device_transfer(&device, tx, rx, sizeof(tx), 10U));
    TEST_ASSERT_EQUAL(GPIO_PIN_SET, hss_fake_gpio_output_state(&cs_port, GPIO_PIN_4));
    TEST_ASSERT_EQUAL(HSS_FAKE_SPI_OP_TRANSFER, hss_fake_spi_state()->operation);
}

static void spi_device_controls_active_high_chip_select(void)
{
    SPI_HandleTypeDef spi = {0};
    GPIO_TypeDef cs_port = {0};
    hss_spi_device_t device = {
        .spi = &spi,
        .cs_port = &cs_port,
        .cs_pin = GPIO_PIN_5,
        .cs_active_high = true,
    };

    TEST_ASSERT_EQUAL(HSS_OK, hss_spi_device_select(&device));
    TEST_ASSERT_EQUAL(GPIO_PIN_SET, hss_fake_gpio_output_state(&cs_port, GPIO_PIN_5));
    TEST_ASSERT_EQUAL(HSS_OK, hss_spi_device_deselect(&device));
    TEST_ASSERT_EQUAL(GPIO_PIN_RESET, hss_fake_gpio_output_state(&cs_port, GPIO_PIN_5));
}

static void spi_device_returns_transfer_error_but_still_deselects(void)
{
    SPI_HandleTypeDef spi = {0};
    GPIO_TypeDef cs_port = {.odr = GPIO_PIN_6};
    uint8_t tx[1] = {0x55U};
    uint8_t rx[1] = {0};
    hss_spi_device_t device = {
        .spi = &spi,
        .cs_port = &cs_port,
        .cs_pin = GPIO_PIN_6,
        .cs_active_high = false,
    };

    hss_fake_spi_set_next_status(HAL_TIMEOUT);
    TEST_ASSERT_EQUAL(HSS_TIMEOUT, hss_spi_device_transfer(&device, tx, rx, sizeof(tx), 10U));
    TEST_ASSERT_EQUAL(GPIO_PIN_SET, hss_fake_gpio_output_state(&cs_port, GPIO_PIN_6));
}

int main(void)
{
    hss_host_fakes_reset();
    UNITY_BEGIN();
    RUN_TEST(spi_rejects_invalid_arguments);
    RUN_TEST(spi_maps_hal_statuses_to_hss_results);
    RUN_TEST(spi_fake_records_transfer_data_and_timeout);
    RUN_TEST(spi_device_controls_active_low_chip_select_around_transfer);
    RUN_TEST(spi_device_controls_active_high_chip_select);
    RUN_TEST(spi_device_returns_transfer_error_but_still_deselects);
    return UNITY_END();
}
