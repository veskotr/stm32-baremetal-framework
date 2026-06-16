#pragma once

#include "hss_result.h"
#include "main.h"

static inline hss_result_t hss_result_from_hal_status(HAL_StatusTypeDef status)
{
    switch (status)
    {
    case HAL_OK:
        return HSS_OK;
    case HAL_ERROR:
        return HSS_ERROR;
    case HAL_BUSY:
        return HSS_BUSY;
    case HAL_TIMEOUT:
        return HSS_TIMEOUT;
    default:
        return HSS_ERROR;
    }
}

