//
// Created by vesko on 17.4.2026 г..
//
#include "main.h"
#include "hss_irq.h"

void hss_irq_enable()
{
    __enable_irq();
}

void hss_irq_disable()
{
    __disable_irq();
}
