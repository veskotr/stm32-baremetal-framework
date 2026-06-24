#include "port.h"

#include "hss_irq.h"

static volatile uint32_t critical_nesting;
static hss_irq_state_t saved_irq_state;

void enter_critical(void)
{
    const hss_irq_state_t state = hss_irq_save();

    if (critical_nesting == 0U)
    {
        saved_irq_state = state;
    }

    ++critical_nesting;
}

void exit_critical(void)
{
    if (critical_nesting == 0U)
    {
        return;
    }

    --critical_nesting;
    if (critical_nesting == 0U)
    {
        hss_irq_restore(saved_irq_state);
    }
}
