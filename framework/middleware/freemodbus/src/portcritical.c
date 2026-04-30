//
// Created by vesko on 17.4.2026 г..
//

#include "hss_irq.h"
#include "port.h"

void enter_critical(void)
{
    hss_irq_disable();
}

void exit_critical(void)
{
    hss_irq_enable();
}
