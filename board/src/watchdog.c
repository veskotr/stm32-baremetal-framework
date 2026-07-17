#include "hss_board_watchdog.h"

#include "hss_board_roles.h"
#include "hss_watchdog.h"

bool hss_watchdog_is_available(void)
{
#if HSS_BOARD_HAS_WATCHDOG
    return true;
#else
    return false;
#endif
}

hss_result_t hss_watchdog_init(void)
{
#if HSS_BOARD_HAS_WATCHDOG && HSS_BOARD_WATCHDOG_KIND_IWDG
    return hss_iwdg_init(&HSS_BOARD_WATCHDOG_HANDLE);
#elif HSS_BOARD_HAS_WATCHDOG && HSS_BOARD_WATCHDOG_KIND_WWDG
    return hss_wwdg_init(&HSS_BOARD_WATCHDOG_HANDLE);
#else
    return HSS_NOT_SUPPORTED;
#endif
}

hss_result_t hss_watchdog_refresh(void)
{
#if HSS_BOARD_HAS_WATCHDOG && HSS_BOARD_WATCHDOG_KIND_IWDG
    return hss_iwdg_refresh(&HSS_BOARD_WATCHDOG_HANDLE);
#elif HSS_BOARD_HAS_WATCHDOG && HSS_BOARD_WATCHDOG_KIND_WWDG
    return hss_wwdg_refresh(&HSS_BOARD_WATCHDOG_HANDLE);
#else
    return HSS_NOT_SUPPORTED;
#endif
}
