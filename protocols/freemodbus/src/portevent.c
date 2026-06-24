#include "mb.h"

static eMBEventType g_queued_event;
static BOOL g_event_in_queue;

BOOL xMBPortEventInit(void)
{
    g_event_in_queue = FALSE;
    return TRUE;
}

BOOL xMBPortEventPost(eMBEventType event)
{
    g_queued_event = event;
    g_event_in_queue = TRUE;
    return TRUE;
}

BOOL xMBPortEventGet(eMBEventType *event)
{
    if (g_event_in_queue)
    {
        *event = g_queued_event;
        g_event_in_queue = FALSE;
        return TRUE;
    }

    return FALSE;
}
