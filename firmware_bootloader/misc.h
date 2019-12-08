#ifndef _MISC_H
#define _MISC_H

#include <stdio.h>
#include "port.h"


#define sbi(x, y) (x)->BSRR = (y)
#define cbi(x, y) (x)->BRR = (y)
#define _BV(x) (1UL << (x))
#define _dbg() xprintf("%d %s\n", __LINE__, __FILE__)

unsigned char ByteReverse(unsigned char byte);
void _delay_us(volatile unsigned long n);
void _delay_ms_loop(volatile unsigned long n);
void _delay_ms_systick(volatile unsigned long n);
void _delay_nms_mm(void);

#define _delay_ms _delay_ms_systick

#endif
