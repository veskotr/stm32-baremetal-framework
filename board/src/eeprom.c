#include "hss_eeprom.h"

#include "hss_board_roles.h"
#include "hss_hal_result.h"
#include "main.h"

#include <string.h>

#if HSS_ENABLE_EEPROM_EMULATION

#define HSS_EEPROM_RECORD_MAGIC 0x4853U
#define HSS_EEPROM_EMPTY_HALFWORD 0xFFFFU
#define HSS_EEPROM_RECORD_SIZE 8U
#define HSS_EEPROM_CHECK_XOR 0xA5A5U

typedef struct
{
    uint16_t key;
    uint16_t value;
} hss_eeprom_cache_entry_t;

typedef struct
{
    uint16_t key;
    uint16_t value;
    uint16_t checksum;
    uint16_t magic;
} hss_eeprom_record_t;

static hss_eeprom_cache_entry_t eeprom_cache[HSS_EEPROM_SLOT_COUNT];
static size_t eeprom_cache_count;
static uint32_t eeprom_next_address;
static bool eeprom_initialized;

static uint16_t hss_eeprom_read_halfword(uint32_t address)
{
    return *(const volatile uint16_t *)address;
}

static uint16_t hss_eeprom_checksum(uint16_t key, uint16_t value)
{
    return (uint16_t)(key ^ value ^ HSS_EEPROM_CHECK_XOR);
}

static uint64_t hss_eeprom_pack_record(uint16_t key, uint16_t value)
{
    const hss_eeprom_record_t record = {
        .key = key,
        .value = value,
        .checksum = hss_eeprom_checksum(key, value),
        .magic = HSS_EEPROM_RECORD_MAGIC,
    };
    uint64_t packed = 0U;

    memcpy(&packed, &record, sizeof(record));
    return packed;
}

static bool hss_eeprom_record_is_empty(uint32_t address)
{
    return hss_eeprom_read_halfword(address) == HSS_EEPROM_EMPTY_HALFWORD &&
           hss_eeprom_read_halfword(address + 2U) == HSS_EEPROM_EMPTY_HALFWORD &&
           hss_eeprom_read_halfword(address + 4U) == HSS_EEPROM_EMPTY_HALFWORD &&
           hss_eeprom_read_halfword(address + 6U) == HSS_EEPROM_EMPTY_HALFWORD;
}

static bool hss_eeprom_record_is_valid(uint32_t address, uint16_t *key, uint16_t *value)
{
    const uint16_t stored_key = hss_eeprom_read_halfword(address);
    const uint16_t stored_value = hss_eeprom_read_halfword(address + 2U);
    const uint16_t stored_checksum = hss_eeprom_read_halfword(address + 4U);
    const uint16_t stored_magic = hss_eeprom_read_halfword(address + 6U);

    if (stored_magic != HSS_EEPROM_RECORD_MAGIC)
    {
        return false;
    }

    if (stored_checksum != hss_eeprom_checksum(stored_key, stored_value))
    {
        return false;
    }

    if (key != NULL)
    {
        *key = stored_key;
    }
    if (value != NULL)
    {
        *value = stored_value;
    }
    return true;
}

static hss_eeprom_cache_entry_t *hss_eeprom_find_cache_entry(uint16_t key)
{
    for (size_t index = 0; index < eeprom_cache_count; ++index)
    {
        if (eeprom_cache[index].key == key)
        {
            return &eeprom_cache[index];
        }
    }

    return NULL;
}

static hss_result_t hss_eeprom_cache_set(uint16_t key, uint16_t value)
{
    hss_eeprom_cache_entry_t *entry = hss_eeprom_find_cache_entry(key);
    if (entry != NULL)
    {
        entry->value = value;
        return HSS_OK;
    }

    if (eeprom_cache_count >= HSS_EEPROM_SLOT_COUNT)
    {
        return HSS_INVALID_ARGUMENT;
    }

    eeprom_cache[eeprom_cache_count].key = key;
    eeprom_cache[eeprom_cache_count].value = value;
    eeprom_cache_count++;
    return HSS_OK;
}

static hss_result_t hss_eeprom_program_halfword(uint32_t address, uint16_t value)
{
#if defined(HSS_BOARD_MCU_FAMILY_STM32F1) && defined(FLASH_TYPEPROGRAM_HALFWORD)
    return hss_result_from_hal_status(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, value));
#else
    (void)address;
    (void)value;
    return HSS_NOT_SUPPORTED;
#endif
}

static hss_result_t hss_eeprom_program_record(uint32_t address, uint16_t key, uint16_t value)
{
#if defined(HSS_BOARD_MCU_FAMILY_STM32G0) && defined(FLASH_TYPEPROGRAM_DOUBLEWORD)
    return hss_result_from_hal_status(
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, hss_eeprom_pack_record(key, value))
    );
#else
    hss_result_t result = hss_eeprom_program_halfword(address, key);
    if (hss_result_is_error(result))
    {
        return result;
    }

    result = hss_eeprom_program_halfword(address + 2U, value);
    if (hss_result_is_error(result))
    {
        return result;
    }

    result = hss_eeprom_program_halfword(address + 4U, hss_eeprom_checksum(key, value));
    if (hss_result_is_error(result))
    {
        return result;
    }

    return hss_eeprom_program_halfword(address + 6U, HSS_EEPROM_RECORD_MAGIC);
#endif
}

static hss_result_t hss_eeprom_erase(void)
{
#if defined(HSS_BOARD_MCU_FAMILY_STM32F1)
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t page_error = 0U;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = HSS_EEPROM_FLASH_ORIGIN;
    erase.NbPages = HSS_EEPROM_FLASH_SIZE / HSS_EEPROM_PAGE_SIZE;

    return hss_result_from_hal_status(HAL_FLASHEx_Erase(&erase, &page_error));
#elif defined(HSS_BOARD_MCU_FAMILY_STM32G0)
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t page_error = 0U;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Banks = FLASH_BANK_1;
    erase.Page = (HSS_EEPROM_FLASH_ORIGIN - FLASH_BASE) / HSS_EEPROM_PAGE_SIZE;
    erase.NbPages = HSS_EEPROM_FLASH_SIZE / HSS_EEPROM_PAGE_SIZE;

    return hss_result_from_hal_status(HAL_FLASHEx_Erase(&erase, &page_error));
#else
    return HSS_NOT_SUPPORTED;
#endif
}

static hss_result_t hss_eeprom_compact(void)
{
    hss_result_t result = hss_eeprom_erase();
    if (hss_result_is_error(result))
    {
        return result;
    }

    uint32_t write_address = HSS_EEPROM_FLASH_ORIGIN;
    for (size_t index = 0; index < eeprom_cache_count; ++index)
    {
        result = hss_eeprom_program_record(write_address, eeprom_cache[index].key, eeprom_cache[index].value);
        if (hss_result_is_error(result))
        {
            return result;
        }
        write_address += HSS_EEPROM_RECORD_SIZE;
    }

    eeprom_next_address = write_address;
    return HSS_OK;
}

static bool hss_eeprom_has_space_for_record(void)
{
    return (eeprom_next_address + HSS_EEPROM_RECORD_SIZE) <=
           (HSS_EEPROM_FLASH_ORIGIN + HSS_EEPROM_FLASH_SIZE);
}

static hss_result_t hss_eeprom_scan(void)
{
    eeprom_cache_count = 0U;
    eeprom_next_address = HSS_EEPROM_FLASH_ORIGIN;

    const uint32_t end_address = HSS_EEPROM_FLASH_ORIGIN + HSS_EEPROM_FLASH_SIZE;
    for (uint32_t address = HSS_EEPROM_FLASH_ORIGIN;
         (address + HSS_EEPROM_RECORD_SIZE) <= end_address;
         address += HSS_EEPROM_RECORD_SIZE)
    {
        if (hss_eeprom_record_is_empty(address))
        {
            eeprom_next_address = address;
            return HSS_OK;
        }

        uint16_t key = 0U;
        uint16_t value = 0U;
        if (hss_eeprom_record_is_valid(address, &key, &value))
        {
            hss_result_t result = hss_eeprom_cache_set(key, value);
            if (hss_result_is_error(result))
            {
                return result;
            }
        }
    }

    eeprom_next_address = end_address;
    return HSS_OK;
}

static hss_result_t hss_eeprom_ensure_initialized(void)
{
    if (eeprom_initialized)
    {
        return HSS_OK;
    }

    return hss_eeprom_init();
}

#endif

bool hss_eeprom_is_available(void)
{
#if HSS_ENABLE_EEPROM_EMULATION && HSS_BOARD_HAS_EEPROM_EMULATION
    return true;
#else
    return false;
#endif
}

hss_result_t hss_eeprom_init(void)
{
#if HSS_ENABLE_EEPROM_EMULATION
    if (!HSS_BOARD_HAS_EEPROM_EMULATION)
    {
        return HSS_NOT_SUPPORTED;
    }

    if (HSS_EEPROM_FLASH_ORIGIN == 0U ||
        HSS_EEPROM_FLASH_SIZE == 0U ||
        HSS_EEPROM_PAGE_SIZE == 0U ||
        HSS_EEPROM_SLOT_COUNT == 0U ||
        HSS_EEPROM_FLASH_SIZE < HSS_EEPROM_RECORD_SIZE ||
        (HSS_EEPROM_FLASH_SIZE % HSS_EEPROM_PAGE_SIZE) != 0U ||
        (HSS_EEPROM_FLASH_ORIGIN % HSS_EEPROM_PAGE_SIZE) != 0U ||
        HSS_EEPROM_SLOT_COUNT > (HSS_EEPROM_FLASH_SIZE / HSS_EEPROM_RECORD_SIZE))
    {
        return HSS_INVALID_STATE;
    }

    hss_result_t result = hss_eeprom_scan();
    if (hss_result_is_error(result))
    {
        return result;
    }

    eeprom_initialized = true;
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_eeprom_format(void)
{
#if HSS_ENABLE_EEPROM_EMULATION
    if (!HSS_BOARD_HAS_EEPROM_EMULATION)
    {
        return HSS_NOT_SUPPORTED;
    }

    hss_result_t result = hss_result_from_hal_status(HAL_FLASH_Unlock());
    if (hss_result_is_error(result))
    {
        return result;
    }

    result = hss_eeprom_erase();
    hss_result_t lock_result = hss_result_from_hal_status(HAL_FLASH_Lock());
    if (hss_result_is_error(result))
    {
        return result;
    }
    if (hss_result_is_error(lock_result))
    {
        return lock_result;
    }

    eeprom_cache_count = 0U;
    eeprom_next_address = HSS_EEPROM_FLASH_ORIGIN;
    eeprom_initialized = true;
    return HSS_OK;
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_eeprom_read_u16(uint16_t key, uint16_t *value)
{
#if HSS_ENABLE_EEPROM_EMULATION
    if (!HSS_BOARD_HAS_EEPROM_EMULATION)
    {
        (void)key;
        (void)value;
        return HSS_NOT_SUPPORTED;
    }

    if (value == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

    hss_result_t result = hss_eeprom_ensure_initialized();
    if (hss_result_is_error(result))
    {
        return result;
    }

    hss_eeprom_cache_entry_t *entry = hss_eeprom_find_cache_entry(key);
    if (entry == NULL)
    {
        return HSS_NOT_READY;
    }

    *value = entry->value;
    return HSS_OK;
#else
    (void)key;
    (void)value;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_eeprom_write_u16(uint16_t key, uint16_t value)
{
#if HSS_ENABLE_EEPROM_EMULATION
    if (!HSS_BOARD_HAS_EEPROM_EMULATION)
    {
        (void)key;
        (void)value;
        return HSS_NOT_SUPPORTED;
    }

    hss_result_t result = hss_eeprom_ensure_initialized();
    if (hss_result_is_error(result))
    {
        return result;
    }

    hss_eeprom_cache_entry_t *entry = hss_eeprom_find_cache_entry(key);
    if (entry != NULL && entry->value == value)
    {
        return HSS_OK;
    }

    hss_result_t unlock_result = hss_result_from_hal_status(HAL_FLASH_Unlock());
    if (hss_result_is_error(unlock_result))
    {
        return unlock_result;
    }

    result = hss_eeprom_cache_set(key, value);
    if (hss_result_is_ok(result))
    {
        if (!hss_eeprom_has_space_for_record())
        {
            result = hss_eeprom_compact();
        }
        if (hss_result_is_ok(result))
        {
            result = hss_eeprom_program_record(eeprom_next_address, key, value);
            if (hss_result_is_ok(result))
            {
                eeprom_next_address += HSS_EEPROM_RECORD_SIZE;
            }
        }
    }

    hss_result_t lock_result = hss_result_from_hal_status(HAL_FLASH_Lock());
    if (hss_result_is_error(result))
    {
        return result;
    }
    if (hss_result_is_error(lock_result))
    {
        return lock_result;
    }

    return HSS_OK;
#else
    (void)key;
    (void)value;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_eeprom_load_u16_array(uint16_t start_key, uint16_t *values, size_t count)
{
    if (values == NULL && count > 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

    for (size_t index = 0; index < count; ++index)
    {
        hss_result_t result = hss_eeprom_read_u16((uint16_t)(start_key + index), &values[index]);
        if (result == HSS_NOT_READY)
        {
            values[index] = 0U;
            continue;
        }
        if (hss_result_is_error(result))
        {
            return result;
        }
    }

    return HSS_OK;
}

hss_result_t hss_eeprom_save_u16_array(uint16_t start_key, const uint16_t *values, size_t count)
{
    if (values == NULL && count > 0U)
    {
        return HSS_INVALID_ARGUMENT;
    }

    for (size_t index = 0; index < count; ++index)
    {
        hss_result_t result = hss_eeprom_write_u16((uint16_t)(start_key + index), values[index]);
        if (hss_result_is_error(result))
        {
            return result;
        }
    }

    return HSS_OK;
}
