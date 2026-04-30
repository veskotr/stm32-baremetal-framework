//
// Created by vesko on 17.4.2026 г..
//

#include "mb.h"

static eMBEventType eQueuedEvent;
static BOOL xEventInQueue;

BOOL xMBPortEventInit(void)
{
    xEventInQueue = FALSE;
    return TRUE;
}

BOOL xMBPortEventPost(eMBEventType eEvent)
{
    eQueuedEvent = eEvent;
    xEventInQueue = TRUE;
    return TRUE;
}

BOOL xMBPortEventGet(eMBEventType *eEvent)
{
    if (xEventInQueue)
    {
        *eEvent = eQueuedEvent;
        xEventInQueue = FALSE;
        return TRUE;
    }
    return FALSE;
}