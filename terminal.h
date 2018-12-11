#ifndef __TERMINAL_H_INCLUDED
#define __TERMINAL_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

void handle_out(uint8_t dev, uint8_t A);
void terminal_clear(void);

#endif
