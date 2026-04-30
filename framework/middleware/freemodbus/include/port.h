//
// Created by vesko on 17.4.2026 г..
//
#ifndef PORT_H
#define PORT_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/* ---- FreeModbus base types ---- */
typedef uint8_t UCHAR;
typedef uint16_t USHORT;
typedef uint32_t ULONG;

typedef char CHAR;
typedef bool BOOL;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define ENTER_CRITICAL_SECTION()   enter_critical()
#define EXIT_CRITICAL_SECTION()    exit_critical()

void enter_critical(void);
void exit_critical(void);

#endif
