/**
 * Extended Memory interface for the Atmega2560 MCU.
 *
 * @author Francisco Soto <francisco@nanosatisfi.com>
 ******************************************************************************/

#include <stdlib.h>
#include <avr/io.h>

#include "conf_xmem.h"
#include "atmega2560-xmem.h"

/* If memory is not a multiple of 64KB we need to find out what's the last bank size. */
#if (XMEM_TOTAL_MEMORY % 65536) == 0
#define XMEM_LAST_BANK_END  ((void *)0xffff)
#else
#define XMEM_LAST_BANK_END  ((void *)((XMEM_TOTAL_MEMORY % 65536) - 1))
#endif

/* Atmega XMEM address space block */
#define XMEM_START      ((void *)0x2200)
#define XMEM_END        ((void *)0xffff)

/* The address space to use for unshadowed memory */
#define XMEM_SHADOWED_START ((void *)0x8000)
#define XMEM_SHADOWED_END   ((void *)0x9fff)

struct bank_heap_state {
    void *__brkval;             /* Pointer between __malloc_heap_start and __malloc_heap_end, shows growth. */
    void *__flp;                /* Pointer to the free block list that malloc handles. */
    char *__malloc_heap_start;  /* Pointer to the beginning of the heap. */
    char *__malloc_heap_end;    /* Pointer to the end of the heap, 0 if the heap is below the stack. */
};

/* Private heap variables */
#ifdef __cplusplus
extern "C" {
#endif
    extern void *__flp;
    extern void *__brkval;
#ifdef __cplusplus
}
#endif

struct bank_heap_state _system_heap_state;
struct bank_heap_state _bank_state[XMEM_BANKS];
uint8_t _system_heap_in_place = 0;
uint8_t _current_bank = -1;

/**
 * @docstring
 * Save the current heap state in the given bank_heap_state pointer.
 */
static void _xmem_save_bank_state (struct bank_heap_state *bs) {
    bs->__brkval = __brkval;
    bs->__flp = __flp;
    bs->__malloc_heap_start = __malloc_heap_start;
    bs->__malloc_heap_end = __malloc_heap_end;
}

/**
 * @docstring
 * Load the given bank_heap_state into the global heap state.
 */
static void _xmem_load_bank_state (struct bank_heap_state *bs) {
    __brkval = bs->__brkval;
    __flp = bs->__flp;
    __malloc_heap_start = bs->__malloc_heap_start;
    __malloc_heap_end = bs->__malloc_heap_end;
}

/**
 * @docstring
 * Unshadow the lower 8KB of the extended memory and return a pointer that
 * you can use to access it. You have to call the xmem_shadow_lower_memory when done.
 * The pointer returned is the start of your 8KB block of memory.
 */
void *xmem_unshadow_lower_memory (void) {
    /* Configures PORTC pins as output pins. */
    DDRC = 0xff;

    /* Port C has all 0s now. */
    PORTC = 0x00;

    /* Release the 5,6,7 pins from extended memory addressing duty. They are still
       addressing memory, they are just always set to 0 and that will leave us with
       only 13 pins (8KB) of address in external memory. Since these pins are zeroed out,
       you will be effectively addressing the lower 8KB of external memory. */
    XMCRB = (1 << XMM0) | (1 << XMM1);

    return XMEM_SHADOWED_START;
}

/**
 * @docstring
 * Shadow the lower 8KB of the extended memory and set normal addressing pins.
 */
void xmem_shadow_lower_memory (void) {
    /* Configures PORTC pins as output pins. */
    DDRC = 0xff;

    /* Port C has all 0s now. */
    PORTC = 0x00;

    /* Set every pin to regular memory addressing duty. */
    XMCRB = 0;
}

/**
 * @docstring
 * Switch bank if the bank exist and is not the current one.
 */
void xmem_switch_bank (uint8_t bank) {
    if (_current_bank == bank || bank > XMEM_BANKS) {
        return;
    }

    if (!_system_heap_in_place) {
        /* Save the current bank heap state */
        _xmem_save_bank_state(&_bank_state[_current_bank]);

        /* And restore the state we are switching to. */
        _xmem_load_bank_state(&_bank_state[bank]);
    }

    _current_bank = bank;

    /* Have the user set the higher bits */
    XMEM_USER_SWITCH_BANK(bank);
}

/**
 * @docstring
 * This will save the current bank state and return the heap to the
 * internal memory. You can effectively stop xmem from being used for
 * the heap if you use this function.
 */
void xmem_set_system_heap (void) {
    if (_system_heap_in_place) {
        return;
    }

    _xmem_save_bank_state(&_bank_state[_current_bank]);
    _xmem_load_bank_state(&_system_heap_state);

    _system_heap_in_place = 1;
}

/**
 * @docstring
 * This should be used after a call to xmem_set_system_heap. This will save
 * the current internal memory heap state and return the heap to the extended
 * memory using the current bank.
 */
void xmem_set_xmem_heap (void) {
    if (!_system_heap_in_place) {
        return;
    }

    _xmem_save_bank_state(&_system_heap_state);
    _xmem_load_bank_state(&_bank_state[_current_bank]);

    _system_heap_in_place = 0;
}

/**
 * @docstring
 * Returns the last valid address in the current selected bank.
 */
void *xmem_get_current_bank_address_start (void) {
    return (void *)_bank_state[_current_bank].__malloc_heap_start;
}

/**
 * @docstring
 * Returns the last valid address in the current selected bank.
 */
void *xmem_get_current_bank_address_end (void) {
    return (void *)_bank_state[_current_bank].__malloc_heap_end;
}

/**
 * @docstring
 * Initializes the external memory and the internal data structures if we are managing the heap in the xmem.
 */
void xmem_init (void) {
   /* TODO: Calculate the # of pins we need for the given memory and
      calculate a mask for XMCRB. Save this mask because we will need it
      to restore XMCRB later on if we want to access the first 8KB of XMEM.
      This assumes we want all 8 bits from PORTC. */
    XMCRB = 0;

    /* XMEM Enable bit, entire xmem is treated like one sector and set the wait
       states for it. */
    XMCRA = (1 << SRE) | (XMEM_WAIT_STATES << SRW10);

    /* Have the user configure his extra pins. */
    XMEM_USER_INIT();

    _xmem_save_bank_state(&_system_heap_state);

    /* If heap should be in xmem ram then we should let avr-libc where is it. */
    __malloc_heap_start = (char *)XMEM_START;
    __malloc_heap_end = (char *)XMEM_END;
    __brkval = (char *)XMEM_START;

#ifdef XMEM_USE_BANKING
    /* All banks except the last one have 64KB size. */
    for (uint8_t i = 0; i < XMEM_BANKS - 1; i++) {
        _xmem_save_bank_state(&_bank_state[i]);
    }
#endif /* XMEM_USE_BANKKING */

    /* Save the last bank with the correct address space size. */
    __malloc_heap_end = (char *)XMEM_LAST_BANK_END;
    _xmem_save_bank_state(&_bank_state[XMEM_BANKS - 1]);

    _system_heap_in_place = 0;

    xmem_switch_bank(0);
}
