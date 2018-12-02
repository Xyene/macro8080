#include "keyboard.h"

static unsigned char ps2_buffer[3];
static unsigned char ascii = 0;
static unsigned char ascii_buffer[4096];

static unsigned char ps2_to_ascii[0x200] = {
        [0x066] = 0x08, // Backspace ("backspace" key)
        [0x166] = 0x08, // Backspace ("backspace" key)
        [0x00d] = 0x09, // Horizontal Tab
        [0x10d] = 0x09, // Horizontal Tab
        [0x05a] = 0x0d, // Carriage return ("enter" key)
        [0x15a] = 0x0d, // Carriage return ("enter" key)
        [0x076] = 0x1b, // Escape ("esc" key)
        [0x176] = 0x1b, // Escape ("esc" key)
        [0x029] = 0x20, // Space
        [0x129] = 0x20, // Space
        [0x116] = 0x21, // !
        [0x152] = 0x22, // "
        [0x126] = 0x23, // #
        [0x125] = 0x24, // $
        [0x12e] = 0x25, //
        [0x13d] = 0x26, //
        [0x052] = 0x27, //
        [0x146] = 0x28, //
        [0x145] = 0x29, //
        [0x13e] = 0x2a, // *
        [0x155] = 0x2b, // +
        [0x041] = 0x2c, // ,
        [0x04e] = 0x2d, // -
        [0x049] = 0x2e, // .
        [0x04a] = 0x2f, // /
        [0x045] = 0x30, // 0
        [0x016] = 0x31, // 1
        [0x01e] = 0x32, // 2
        [0x026] = 0x33, // 3
        [0x025] = 0x34, // 4
        [0x02e] = 0x35, // 5
        [0x036] = 0x36, // 6
        [0x03d] = 0x37, // 7
        [0x03e] = 0x38, // 8
        [0x046] = 0x39, // 9
        [0x14c] = 0x3a, // :
        [0x04c] = 0x3b, // ;
        [0x141] = 0x3c, // <
        [0x055] = 0x3d, // =
        [0x149] = 0x3e, // >
        [0x14a] = 0x3f, // ?
        [0x11e] = 0x40, // @
        [0x11c] = 0x41, // A
        [0x132] = 0x42, // B
        [0x121] = 0x43, // C
        [0x123] = 0x44, // D
        [0x124] = 0x45, // E
        [0x12b] = 0x46, // F
        [0x134] = 0x47, // G
        [0x133] = 0x48, // H
        [0x143] = 0x49, // I
        [0x13b] = 0x4a, // J
        [0x142] = 0x4b, // K
        [0x14b] = 0x4c, // L
        [0x13a] = 0x4d, // M
        [0x131] = 0x4e, // N
        [0x144] = 0x4f, // O
        [0x14d] = 0x50, // P
        [0x115] = 0x51, // Q
        [0x12d] = 0x52, // R
        [0x11b] = 0x53, // S
        [0x12c] = 0x54, // T
        [0x13c] = 0x55, // U
        [0x12a] = 0x56, // V
        [0x11d] = 0x57, // W
        [0x122] = 0x58, // X
        [0x135] = 0x59, // Y
        [0x11a] = 0x5a, // Z
        [0x054] = 0x5b, // [
        [0x05d] = 0x5c, // Backslash
        [0x05b] = 0x5d, // ]
        [0x136] = 0x5e, // ^
        [0x14e] = 0x5f, // _
        [0x00e] = 0x60, // `
        [0x01c] = 0x61, // a
        [0x032] = 0x62, // b
        [0x021] = 0x63, // c
        [0x023] = 0x64, // d
        [0x024] = 0x65, // e
        [0x02b] = 0x66, // f
        [0x034] = 0x67, // g
        [0x033] = 0x68, // h
        [0x043] = 0x69, // i
        [0x03b] = 0x6a, // j
        [0x042] = 0x6b, // k
        [0x04b] = 0x6c, // l
        [0x03a] = 0x6d, // m
        [0x031] = 0x6e, // n
        [0x044] = 0x6f, // o
        [0x04d] = 0x70, // p
        [0x015] = 0x71, // q
        [0x02d] = 0x72, // r
        [0x01b] = 0x73, // s
        [0x02c] = 0x74, // t
        [0x03c] = 0x75, // u
        [0x02a] = 0x76, // v
        [0x01d] = 0x77, // w
        [0x022] = 0x78, // x
        [0x035] = 0x79, // y
        [0x01a] = 0x7a, // z
        [0x154] = 0x7b, // {
        [0x15d] = 0x7c, // |
        [0x15b] = 0x7d, // }
        [0x10e] = 0x7e, // ~
        [0x071] = 0x7f, // (Delete OR DEL on numeric keypad)
        [0x171] = 0x7f, // (Delete OR DEL on numeric keypad)
};

static int is_shift = 0;
static char last_input = 0;

uint8_t handle_in(uint8_t dev) {
  switch (dev) {
    case 16:
      return has_keyboard_input() ? 0x03 : 0x02;
    case 17: {
      char ret = has_keyboard_input() ? read_keyboard_input() : '\0';
      return ret;
    }
    case 255:
      return 0;
    default:
      printf("Invalid device read %02X\n", dev);
      return 0;
  }
}

char read_keyboard_input(void) {
#ifdef OVERRIDE_INPUT
  if (*input_override) {
    char ret = *input_override;
    input_override++;
    return ret;
  }
#endif

#ifdef __unix__
  char ret = toupper(getchar());
  return ret == '\n' ? '\r' : ret;
#else
  char ret = toupper(last_input);
  last_input = 0;
  return ret;
#endif
}

int has_keyboard_input(void) {
#ifdef OVERRIDE_INPUT
  if (*input_override)
    return 1;
#endif

#ifdef __unix__
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10;

  fd_set fds;
  FD_SET(fileno(stdin), &fds);

  if (select(fileno(stdin) + 1, &fds, NULL, NULL, &timeout) == -1) {
    return 0;
  }

  return FD_ISSET(fileno(stdin), &fds);
#else
  if (last_input)
    return 1;

  volatile int *PS2 = (int *)0xFF200100;
  while ((*PS2) & 0x8000 /* RVALID */) {
    ps2_buffer[0] = ps2_buffer[1];
    ps2_buffer[1] = ps2_buffer[2];
    ps2_buffer[2] = (*PS2) & 0xFF;

    if (ps2_buffer[1] == 0xAA && ps2_buffer[2] == 0x00) {
      printf("Inserted keyboard!\n");
    } else {
      // printf("Code %02X %02X %02X\n", ps2_buffer[0], ps2_buffer[1],
      //	   ps2_buffer[2]);
    }

    int shift_pressed_now = 0;
    switch (ps2_buffer[2]) {
      case 0x00: // Key detection error or internal buffer overrun
      case 0xEE: // Response to echo command
      case 0xFA: // Command ACK
      case 0xFC: // Self test failed
      case 0xFD: // ^^
      case 0xFE: // Resend request
      case 0xFF: // Key detection error
        printf("Keyboard notification %02X\n", ps2_buffer[2]);
        continue;
      case 0xAA: // Self test passed or keyboard connected
        printf("Keyboard connected!\n");
        continue;
      case 0x59:
      case 0x12:
        is_shift = 1;
        shift_pressed_now = 1;
        break;
    }

    switch (ps2_buffer[1]) {
      case 0xF0:
        if (shift_pressed_now) {
          // printf("Shift depressed\n");
          is_shift = 0;
        } else {
          char c = ps2_to_ascii[ps2_buffer[2] | (is_shift << 8)];
          // printf("	Typed '%c' (%02X)\n", c, (int) c);
          last_input = c;
          return 1;
        }
        break;
      default: // There's a bunch more codes that I don't think are necessary
        break;
    }
  }

  return 0;
#endif
}
