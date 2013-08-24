atmega2560-xmem
========

This is a library for extended memory support on the Atmega2560 microcontroller. This MCU
has 8KB of memory by default but it supports external memory if you want to add more memory to
your board.

This library supports banking so you can use memory higher than 64KB, the atmega2560 address space
limit, and also will configure avr-libc malloc()/free() to use the external memory for the heap and
leave the internal 8KB of RAM for stack space.

## Requirements

* [avr-libc]

# Functions

`void xmem_init (void)`

Initializes the Atmega2560 external memory interface and calls a user defined initialization code.

`void xmem_switch_bank (uint8_t bank)`

Switches between banks when more than one bank is available. If heap management is enabled it will
also save and restore the banks heap configuration.

# Configuration

You can, and must, configure the behavior of this code by changing some `#define` statements in the
conf_xmem.h file. These definitions are:

`#define XMEM_TOTAL_MEMORY  131072`

Total amount of external memory installed in your board. This number is in bytes.

`#define XMEM_USER_INIT() ((void) 0)`

If your board has special initialization required, you _must_ define the code here before compiling
the library. This may be involved initializing extra pins for bank selection or chip enable pins.

`#define XMEM_USER_SWITCH_BANK(bank_) ((void) 0)`

If you have more than 64KB then banking is needed. That means you have extra pins set up for the upper
address bits on your external memory chip. Since we do not know beforehand which pins you are going to
use you have to tell the library using this define. The macro will receive the a bank_ variable and it
has to properly select the bank given this parameter. Check conf_xmem.h for some examples of it.

`#define XMEM_WAIT_STATES  0`

Some memory chips have timing requirements that must be met in order to function properly, if you have
one of those chips you can configure the wait states with this define. The values allowed are:

- 0 = No wait-states.
- 1 = Wait one cycle during read/write strobe.
- 2 = Wait two cycles during read/write strobe.
- 3 = Wait two cycles during read/write and wait one cycle before driving out new address */

`#define XMEM_HEAP_IN_XMEM`

Define this if you want avr-libc malloc()/free() heap allocation functions to use extended memory.
Proper state will be maintained between banks for you by the library. You just have to switch banks
if you ran out of memory in the one you are currently in.

Please note that this will leave the internal mcu ram for stack space only unless you do some sorcery
of your own.
