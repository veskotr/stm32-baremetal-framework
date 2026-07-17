#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "hss_result.h"

bool hss_console_is_available(void);
hss_result_t hss_console_write_bytes(const uint8_t *data, uint16_t length, uint32_t timeout_ms);
hss_result_t hss_console_write(const char *text, uint32_t timeout_ms);
hss_result_t hss_console_write_line(const char *text, uint32_t timeout_ms);
hss_result_t hss_console_read_bytes(uint8_t *data, uint16_t length, uint32_t timeout_ms);
hss_result_t hss_console_read_byte(uint8_t *byte, uint32_t timeout_ms);
