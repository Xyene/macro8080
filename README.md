# macro8080

This repository houses an implementation of an Intel 8080 microprocessor in C, which can run Altair BASIC and several CP/M test ROMs.

<img src="https://i.imgur.com/xz9QXcG.jpg" width="400px"></img>

It's main target is a Nios II processor (specifically, an Altera DE1-SoC), though it contains shims for full functionality in Unix environments. A barebones PS/2 keyboard driver is implemented for input when running on the DE1-SoC.

This project was mostly intended as an experiment to see whether the core of an emulator could be implemented entirely using macros, as opposed to modularized code segments. Intuitively, this would provide greater performance at the cost of code size, which empirically appears to be valid.

## Compilation
To build for a DE1-SoC, just import `basic.amp`.

To compile and run on a Unix machine:

```
$ git clone https://github.com/Xyene/macro8080.git
$ cd macro8080
$ make
$ ./i8080
```

By default, 8K Altair BASIC will load. Several macro definitions control further behaviour if macro8080 is built with them:

* `CPM` will load a CPM test ROM and build CPM BIOS functions
* `OVERRIDE_INPUT` will automatically feed the source code of Civil War into BASIC (useful for demoing on a DE1-SoC)

## How does it work?
A short write-up about the methods used to implement the Intel 8080 core entirely in macros is given below.

### Dispatching instructions
A typical Intel 8080 instruction is encoded as an 8-bit value, with register operands embedded in the opcode. The encoding for
all `MOV` instructions is shown below.

```
01 aaa bbb
|   |   └── source register
|   └── destination register
└── MOV prefix   
```

For example, `MOV A, B` is encoded as `01 111 000`; `111` identifies register `A`, and `000` identifies register `B`.
In pseudo-code, a possible implementation of `MOV` looks like this:

```c
void mov(uint8_t opcode) {
  uint8_t src = opcode & 0x7;
  uint8_t dst = (opcode >> 3) & 0x7;
  set_reg(dst, get_reg(src));  // set_reg and get_reg are switch-based lookups
}
```

This is clean code, but is problematic in that there's a large overhead (in terms of cycles on the host machine) for the `set_reg` and `get_reg` routines that can't easily be eliminated... but what if we used macros to generate code for all possible permutations of an instruction?

Illustrating with an example, `MOV` can be implemented like:

```c
#define MOV(X, Y) \
    X = Y;
```

We can combine this with a lookup table for fast dispatches (GCC allows taking the address of labels).

```c
#define DONE goto done_opcode;
#define MOV(X, Y) \
  X = Y;          \
  DONE

void run_forever(void) {
  // Declare all registers, memory, etc.
  ...
  
  static void *ops[] = {
    ... &&MOV_A_B, &&MOV_A_C, &&MOV_A_D, ...
  };

  while (1) {
    goto *ops[memory[PC++]];
    done_opcode: ;
  }

  ...
  MOV_A_B:
    MOV(A, B)
  MOV_A_C:
    MOV(A, C)
  MOV_A_D:
    MOV(A, D)
  ...
}
```

You can [check out this example on godbolt.org](https://gcc.godbolt.org/z/UCntF9) to view a colorized disassembly. It's worth noting that in the end, the greatest overhead for our `MOV` instructions becomes `goto *ops[memory[PC++]]`, which is an operation we'd have to perform regardless of how our opcode was implemented &mddash; good!

### More plumbing
The Intel 8080 is an 8-bit processor, but can work on 16-bit data in the form of "register pairs". Register pairs are 16-bit values composed of two 8-bit registers. Valid ones are `BC`, `DE`, and `HL`.

If we want to be able to use macros everywhere, we need some efficient way to have e.g. the statement `B = N` set the upper 8 bits of `BC` to `N`. A simple solution here is to make use of a union of two 8-bit values and a 16-bit value, and some macros to hide the underlying complexity.

```c
typedef union {
  uint16_t pair;
  uint8_t reg[2];
} regpair_t;

register uint8_t A;
register regpair_t bc, de, hl;

#define B bc.reg[1]
#define C bc.reg[0]
#define BC bc.pair
#define D de.reg[1]
#define E de.reg[0]
#define DE de.pair
#define H hl.reg[1]
#define L hl.reg[0]
#define HL hl.pair
#define M memory[HL]
```

Thankfully, [GCC does a good job at optimizing these accesses](https://gcc.godbolt.org/z/CXjK7G).
