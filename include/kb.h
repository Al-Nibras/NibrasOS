#ifndef __KB_H
#define __KB_H

#include <system.h>

#define NULL 0
#define KB_DATA_REG			0x60
#define KB_CTRL_REG			0x64

extern unsigned int write_port(unsigned int _port,unsigned int _val);
extern unsigned int read_port(unsigned int _port);
extern short test_PS2_controller();
void wait_for_input_buffer();
unsigned char kb_read_status();
void kb_send_cmd(unsigned char cmd);
void kb_encoder_send_cmd(unsigned char cmd);
void kb_enable();
void kb_disable();
void kkybrd_set_leds(char num_led, char caps_led, char scroll_led);


enum KB_STATUS_MASK {

	KB_STATUS_MASK_OUT_BUF	=	0x1,		//00000001
	KB_STATUS_MASK_IN_BUF		=	0x2,		//00000010
	KB_STATUS_MASK_SYSTEM		=	0x4,		//00000100
	KB_STATUS_MASK_CMD_DATA	=	0x8,		//00001000
	KB_STATUS_MASK_LOCKED		=	0x10,		//00010000
	KB_STATUS_MASK_AUX_BUF	=	0x20,		//00100000
	KB_STATUS_MASK_TIMEOUT	=	0x40,		//01000000
	KB_STATUS_MASK_PARITY		=	0x80		//10000000
};

/* Note: if bit 7 of scancode is set, 'scancode & 0x80' 0x80=1000 0000b, then the user just released the key */
static const unsigned char st_keymap[128] = {
		  0,
		  27,
		 '1',
		 '2',
		 '3',
		 '4',
		 '5',
		 '6',
		 '7',
		 '8',
		 '9',
		 '0',
		 '-',
		 '=',
		 '\b',	/* Backspace */
		 '\t',			/* Tab */
		 'q', 'w', 'e', 'r','t', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
		  0,	/* Control */
		 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
		 '\'', '`',
		  0,	/* Left shift */
	     '\\', 'z', 'x', 'c', 'v', 'b', 'n',
		 'm', ',', '.', '/',
		  0,	/* Right shift */
		 '*',
		  0,	/* Alt */
		 ' ',	/* Space bar */
		  0,	/* Caps lock */
		  0,	/* 59 - F1 key ... > */
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,	/* < ... F10 */
		  0,	/* 69 - Num lock*/
		  0,	/* Scroll Lock */
		  0,	/* Home key */
		  0,	/* Up Arrow */
		  0,	/* Page Up */
		 '-',
		  0,	/* Left Arrow */
		  0,
		  0,	/* Right Arrow */
		 '+',
		  0,	/* 79 - End key*/
		  0,	/* Down Arrow */
		  0,	/* Page Down */
		  0,	/* Insert Key */
		  0,	/* Delete Key */
		  0,   0,   0,
		  0,	/* F11 Key */
		  0,	/* F12 Key */
		  0,	/* All other keys are undefined */
};

static const unsigned char apple_macbook_keymap[160] = {
	'A','S','D','F','H','G','Z','X','C','V', NULL,
	'B','Q','W','E','R','T','Y',NULL,
	'1','2','3','4','6','5','+','9','7','-','8','0',
	']','O','U','[','I','P','\r' /* return */,'L','J','\'','K',';','\\', ',','/','N','M','.','\t',' ',
		'~','\b',NULL,
			NULL /* actually it's esc */,
			NULL,
			NULL /* it's cmd */,
			NULL /* l-shift */,
			NULL /* caps lock */,
			NULL /* l-alt */,
			NULL /* l-ctrl */,
			NULL /* r-shift */,
			NULL /*r-alt*/,
			NULL,
			NULL /* fn */,
			NULL /* F13 */,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			'\r', /* another return */
			NULL,
			NULL,
			NULL /* F18 */,
			NULL /* F19 */,
			NULL /* F20 */,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,

			NULL /* F5 */,
			NULL /* F6 */,
			NULL /* F7 */,
			NULL /* F3 */,
			NULL /* F8 */,
			NULL /* F9 */,
			NULL,
			NULL /* F11 */,
			NULL,
			NULL /* F13 */,
			NULL /* F16 */,
			NULL /* F1 */,
			NULL,
			NULL /* F10 */,
			NULL,
			NULL /* F12 */,
			NULL,
			NULL /* F2 */,
			NULL,
			NULL /* home */,
			NULL /* page-up */,
			NULL,
			NULL /* F4 */,
			NULL /* end */,
			NULL /* F2 */,
			NULL /* page-down */,
			NULL /* F1 */,

			NULL /* left-arrow */,
			NULL /* right-arrow */,
			NULL /* bottom-arrow */,
			NULL /* up-arrow */,

			NULL,
			NULL,
			NULL,
			NULL,
			NULL /* F4 */,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL /* F3 */
			/*
				Numpad 1	key code 83
				Numpad 2	key code 84
				Numpad 3	key code 85
				Numpad 4	key code 86
				Numpad 5	key code 87
				Numpad 6	key code 88
				Numpad 7	key code 89
				Numpad 8	key code 91
				Numpad 9	key code 92
				Numpad 0	key code 82
				Numpad *	key code 67
				Numpad /	key code 75
				Numpad +	key code 69
				Numpad -	key code 78
				Numpad =	key code 81
				Numpad .	key code 65
				Numpad clear	key code 71
			*/
};


static const unsigned char st_shiftmap[128] = {
		  0,
		  27,
		 '!',
		 '@',
		 '#',
		 '$',
		 '%',
		 '^',
		 '&',
		 '*',
		 '(',
		 ')',
		 '_',
		 '+',
		 '\b',	/* Backspace */
		 '\t',			/* Tab */
		 'Q', 'W', 'E', 'R','T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
		  0,	/* Control */
		 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
		 '\"', '~',
		  0,	/* Left shift */
	     '|', 'Z', 'X', 'C', 'V', 'B', 'N',
		 'M', '<', '>', '?',
		  0,	/* Right shift */
		 '*',
		  0,	/* Alt */
		 ' ',	/* Space bar */
		  0,	/* Caps lock */
		  0,	/* 59 - F1 key ... > */
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,	/* < ... F10 */
		  0,	/* 69 - Num lock*/
		  0,	/* Scroll Lock */
		  0,	/* Home key */
		  0,	/* Up Arrow */
		  0,	/* Page Up */
		 '-',
		  0,	/* Left Arrow */
		  0,
		  0,	/* Right Arrow */
		 '+',
		  0,	/* 79 - End key*/
		  0,	/* Down Arrow */
		  0,	/* Page Down */
		  0,	/* Insert Key */
		  0,	/* Delete Key */
		  0,   0,   0,
		  0,	/* F11 Key */
		  0,	/* F12 Key */
		  0,	/* All other keys are undefined */
};

#endif /* __KB_H */
