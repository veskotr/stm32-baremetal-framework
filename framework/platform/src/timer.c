#include "hss_timer.h"
#include "tim.h"
#include "board_config.h"
#include <stddef.h>


#if BOARD_TIM2
extern TIM_HandleTypeDef htim2;
#endif

typedef void (*hss_timer_cb_t)(void);

typedef struct
{
    TIM_HandleTypeDef* htim;
    hss_timer_cb_t cb;
} hss_timer_entry_t;

#if BOARD_TIM2
static hss_timer_entry_t tim2_entry =
{
    .htim = &htim2,
    .cb = NULL
};
#endif


static hss_timer_entry_t* timer_map[BOARD_TIMER_COUNT] = {0};


static void hss_timer_init_map(void)
{
#if BOARD_TIM2
    timer_map[HSS_TIMER2] = &tim2_entry;
#endif
}


static hss_timer_entry_t* get_entry(hss_timer_id_t id)
{
    if (id >= BOARD_TIMER_COUNT)
        return NULL;

    return timer_map[id];
}

void hss_timer_init(void)
{
#if BOARD_TIM2
    MX_TIM2_Init();
#endif

    hss_timer_init_map();
}

void hss_timer_start_it(hss_timer_id_t timer)
{
    hss_timer_entry_t* e = get_entry(timer);
    if (!e || !e->htim) return;

    __HAL_TIM_CLEAR_FLAG(e->htim, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(e->htim);
}

void hss_timer_stop_it(hss_timer_id_t timer)
{
    hss_timer_entry_t* e = get_entry(timer);
    if (!e || !e->htim) return;

    HAL_TIM_Base_Stop_IT(e->htim);
    __HAL_TIM_CLEAR_FLAG(e->htim, TIM_FLAG_UPDATE);
    __HAL_TIM_SET_COUNTER(e->htim, 0);
}

void hss_timer_set_cb(hss_timer_id_t timer, hss_timer_cb_t cb)
{
    hss_timer_entry_t* e = get_entry(timer);
    if (!e) return;

    e->cb = cb;
}

static void hss_timer_dispatch(TIM_HandleTypeDef* htim)
{
    for (int i = 0; i < BOARD_TIMER_COUNT; i++)
    {
        hss_timer_entry_t* e = timer_map[i];

        if (e && e->htim == htim)
        {
            if (e->cb)
                e->cb();

            return;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    hss_timer_dispatch(htim);
}
