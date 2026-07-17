#include "hss_watchdog.h"

#include <stddef.h>

#include "hss_hal_result.h"

hss_result_t hss_iwdg_init(IWDG_HandleTypeDef *watchdog)
{
    if (watchdog == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_IWDG_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_IWDG_Init(watchdog));
#else
    (void)watchdog;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_iwdg_refresh(IWDG_HandleTypeDef *watchdog)
{
    if (watchdog == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_IWDG_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_IWDG_Refresh(watchdog));
#else
    (void)watchdog;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_wwdg_init(WWDG_HandleTypeDef *watchdog)
{
    if (watchdog == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_WWDG_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_WWDG_Init(watchdog));
#else
    (void)watchdog;
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_wwdg_refresh(WWDG_HandleTypeDef *watchdog)
{
    if (watchdog == NULL)
    {
        return HSS_INVALID_ARGUMENT;
    }

#ifdef HAL_WWDG_MODULE_ENABLED
    return hss_result_from_hal_status(HAL_WWDG_Refresh(watchdog));
#else
    (void)watchdog;
    return HSS_NOT_SUPPORTED;
#endif
}
