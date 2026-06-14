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

