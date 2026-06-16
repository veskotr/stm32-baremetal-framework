#include "port.h"

#include "hss_irq.h"

void enter_critical(void)
{
    hss_irq_disable();
}

void exit_critical(void)
{
    hss_irq_enable();
}
