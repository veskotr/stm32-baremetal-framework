//
// Created by vesko on 18.4.2026 г..
//
#pragma once

typedef enum
{
    HSS_TIMER1 = 0,
    HSS_TIMER2 = 1,
    HSS_TIMER3 = 2,
} hss_timer_id_t;

typedef void (*hss_timer_cb_t)(void);


void hss_timer_init(void);
void hss_timer_start_it(hss_timer_id_t timer);
void hss_timer_stop_it(hss_timer_id_t timer);
void hss_timer_start(hss_timer_id_t timer);
void hss_timer_stop(hss_timer_id_t timer);
void hss_timer_set_cb(hss_timer_id_t timer, hss_timer_cb_t cb);
