#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    HSS_OK = 0,
    HSS_ERROR = 1,
    HSS_INVALID_ARGUMENT = 2,
    HSS_TIMEOUT = 3,
    HSS_BUSY = 4,
    HSS_NOT_READY = 5,
    HSS_NOT_SUPPORTED = 6,
    HSS_INVALID_STATE = 7,
} hss_result_t;

static inline bool hss_result_is_ok(hss_result_t result)
{
    return result == HSS_OK;
}

static inline bool hss_result_is_error(hss_result_t result)
{
    return result != HSS_OK;
}

static inline const char *hss_result_to_string(hss_result_t result)
{
    switch (result)
    {
    case HSS_OK:
        return "ok";
    case HSS_ERROR:
        return "error";
    case HSS_INVALID_ARGUMENT:
        return "invalid_argument";
    case HSS_TIMEOUT:
        return "timeout";
    case HSS_BUSY:
        return "busy";
    case HSS_NOT_READY:
        return "not_ready";
    case HSS_NOT_SUPPORTED:
        return "not_supported";
    case HSS_INVALID_STATE:
        return "invalid_state";
    default:
        return "unknown";
    }
}
