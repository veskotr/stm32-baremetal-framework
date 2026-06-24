#pragma once

#include <stdint.h>

typedef uint32_t hss_irq_state_t;

void hss_irq_enable(void);
void hss_irq_disable(void);
hss_irq_state_t hss_irq_save(void);
void hss_irq_restore(hss_irq_state_t state);
