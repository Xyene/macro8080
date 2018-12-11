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
A short write-up about the methods used to implement the Intel 8080 core entirely in macros [can be found on my blog](https://tbrindus.ca/emulating-microprocessors-with-macros/).
