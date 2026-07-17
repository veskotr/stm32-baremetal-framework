#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hss_result.h"

bool hss_eeprom_is_available(void);
hss_result_t hss_eeprom_init(void);
hss_result_t hss_eeprom_format(void);
hss_result_t hss_eeprom_read_u16(uint16_t key, uint16_t *value);
hss_result_t hss_eeprom_write_u16(uint16_t key, uint16_t value);
hss_result_t hss_eeprom_load_u16_array(uint16_t start_key, uint16_t *values, size_t count);
hss_result_t hss_eeprom_save_u16_array(uint16_t start_key, const uint16_t *values, size_t count);
