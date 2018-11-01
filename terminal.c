#define VGA_PIXEL_BASE_ADDR	0x8000000
#define VGA_CHAR_BASE_ADDR	0x9000000
#define VGA_WIDTH			320
#define	VGA_HEIGHT			240
// Edge characters not very visible, so just use -1 as limits
#define TERM_WIDTH			79
#define TERM_HEIGHT			59  
#define BLACK				0x0000
#define WHITE				0xFFFF

static int term_x = 0;
static int term_y = 0;

void write_pixel(int x, int y, short colour) {
	volatile short *vga_addr = (volatile short *)(VGA_PIXEL_BASE_ADDR + (y << 10) + (x << 1));
	*vga_addr = colour;
}

void write_char(int x, int y, char c) {
	volatile char *char_addr = (volatile char *)(VGA_CHAR_BASE_ADDR + (y << 7) + x);
	*char_addr = c;
}

char char_at(int x, int y) {
	volatile char *char_addr = (volatile char *)(VGA_CHAR_BASE_ADDR + (y << 7) + x);
	return *char_addr;
}

void terminal_scroll_buffer(void) {
	for (int y = 0; y < TERM_HEIGHT - 1; y++) {
		for (int x = 0; x < TERM_WIDTH; x++) {
			write_char(x, y, char_at(x, y + 1));
		}
	}

	for (int x = 0; x < TERM_WIDTH; x++) {
		write_char(x, TERM_HEIGHT - 1, ' ');
	}
}

void terminal_clear(void) {
	for (int x = 0; x < VGA_WIDTH; x++) {
		for (int y = 0; y < VGA_HEIGHT; y++) {
			write_pixel(x, y, BLACK);
		}
	}

	for (int x = 0; x < TERM_WIDTH + 1; x++) {
		for (int y = 0; y < TERM_HEIGHT + 1; y++) {
			write_char(x, y, ' ');
		}
	}

	term_x = 0;
	term_y = 0;
}

void terminal_out(char c) {
	if (c == '\r' || term_x == TERM_WIDTH) {
		term_x = 0;
		term_y++;

		if (term_y >= TERM_HEIGHT) {
			terminal_scroll_buffer();
			term_y--;
		}
	}

	if (c != '\r') {
		write_char(term_x, term_y, c);
		term_x++;
	}
}

int _main(void) {
	terminal_clear();

	while (1) {
		char x = 'a';
		while (x) {
			terminal_out(x);
			x++;
			for (int i = 0; i < 100000; i++) {
				asm("");
			}
		}
	}

	return 0;
}
