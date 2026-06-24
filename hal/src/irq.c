#include "hss_irq.h"
#include "main.h"

void hss_irq_enable(void)
{
    __enable_irq();
}

void hss_irq_disable(void)
{
    __disable_irq();
}

hss_irq_state_t hss_irq_save(void)
{
    const hss_irq_state_t state = __get_PRIMASK();
    __disable_irq();
    return state;
}

void hss_irq_restore(hss_irq_state_t state)
{
    __set_PRIMASK(state);
}
