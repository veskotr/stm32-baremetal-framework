#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t UCHAR;
typedef uint16_t USHORT;
typedef uint32_t ULONG;

typedef char CHAR;
typedef bool BOOL;

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#define ENTER_CRITICAL_SECTION() enter_critical()
#define EXIT_CRITICAL_SECTION() exit_critical()

void enter_critical(void);
void exit_critical(void);
