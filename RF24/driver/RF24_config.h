#ifndef __RF24_CONFIG_H__
#define __RF24_CONFIG_H__

#include <stddef.h>
#include "mbed.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define _BV(x) (1<<(x))

#undef SERIAL_DEBUG
#ifdef SERIAL_DEBUG
#define IF_SERIAL_DEBUG(x) ({x;})
#else
#define IF_SERIAL_DEBUG(x)
#endif

// Avoid spurious warnings
#if 1
#if ! defined( NATIVE ) && defined( ARDUINO )
#undef PROGMEM
#define PROGMEM __attribute__(( section(".progmem.data") ))
#undef PSTR
#define PSTR(s) (__extension__({static const char __c[] PROGMEM = (s); &__c[0];}))
#endif
#endif

// Progmem is Arduino-specific
#ifdef ARDUINO
#include <avr/pgmspace.h>
#define PRIPSTR "%S"
#else
// typedef char const char;
typedef uint16_t prog_uint16_t;
#define PSTR(x) (x)
#define printf_P printf
#define strlen_P strlen
// #define PROGMEM
// #define pgm_read_word(p) (*(p)) 
#define PRIPSTR "%s"
#endif

#endif // __RF24_CONFIG_H__
// vim:ai:cin:sts=2 sw=2 ft=cpp
