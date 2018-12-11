#pragma once

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __unix__
#include <sys/select.h>
#include <termio.h>
#include <time.h>
#include <unistd.h>
#endif

#ifdef OVERRIDE_INPUT
#include "input_civilwar.h"
#endif

uint8_t handle_in(uint8_t dev);
char read_keyboard_input(void);
int has_keyboard_input(void);
