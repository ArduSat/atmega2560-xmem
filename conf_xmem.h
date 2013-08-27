/**
 * Extended Memory interface for the Atmega2560 MCU.
 *
 * Configuration file.
 *
 * @author Francisco Soto <francisco@nanosatisfi.com>
 ******************************************************************************/

#ifndef CONF_XMEM_H_INCLUDED
#define CONF_XMEM_H_INCLUDED

/* Total amount of your external ram (bytes). */
#define XMEM_TOTAL_MEMORY  131072

/* This is the initialization needed for the Megaram (128KB) shield for Arduino Mega 2560.
   PL7 is RAM chip enable, active high. PD7 is BANKSEL. */
#define XMEM_EXAMPLE_MEGARAM_USER_INIT() \
    DDRD |= _BV(7);                      \
    PORTD = 0x7F;                        \
    DDRL |= _BV(7);                      \
    PORTL = 0xFF;

/* This is the bank switch needed for the Megaram (128KB) shield for Arduino Mega 2560.
   PD7 is bank selector so we only have to set one bit to 0 or 1. */
#define XMEM_EXAMPLE_MEGARAM_USER_SWITCH_BANK(bank_) \
    PORTD &= 0x7F | ((bank_ & 1) << 7);

/* Does your board have special initialization?
   This only applies if you have mroe than 64KB of external memory and want
   to initialize the pins that will be used for banking. Check XMEM_EXAMPLE_MEGARAM_USER_INIT. */
#define XMEM_USER_INIT() ((void) 0)

/* More than 64KB ? Then you need banks and you need to provide the library
   with this macro so it can switch banks and manage them for you, this usually
   requires setting the higher address bits with the pins you wired in your
   board, if you are using 64KB or less, just leave it void..
   Check XMEM_EXAMPLE_MEGARAM_USER_SWITCH_BANK.*/
#define XMEM_USER_SWITCH_BANK(bank_) ((void) 0)

/* Does your memory require you to wait? Let me know about it!
   0 = No wait-states.
   1 = Wait one cycle during read/write strobe.
   2 = Wait two cycles during read/write strobe.
   3 = Wait two cycles during read/write and wait one cycle before driving out new address */
#define XMEM_WAIT_STATES  0

#endif /* CONF_XMEM_H_INCLUDED */
