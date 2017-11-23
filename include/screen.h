#ifndef __SCREEN_H
#define __SCREEN_H

#include <system.h>
#include <string.h>

#define BLACK 0x00
#define BLUE 0x01
#define GREEN 0x02
#define CYAN 0x03
#define RED 0x04
#define MAGENTA 0x05
#define BROWN 0x06
#define LIGHT_GREY 0x07
#define DARK_GREY 0x08
#define LIGHT_BLUE 0x09
#define LIGHT_GREEN 0x0A
#define LIGHT_CYAN 0x0B
#define LIGHT_RED 0x0C
#define LIGHT_MAGENTA 0x0D
#define YELLOW 0X0E
#define WHITE 0X0F

#define VIDEO_MEMORY_ADDRESS 0xb8000 //up to 0xbffff : 32KB
extern unsigned short* vram_up, *vram_up_start, *vram_down, *vram_down_start;
typedef struct {
    unsigned char attributes;
    int x;
    int y;
} console;
/*
 * Calculates the attribute byte for a foreground and a background color.
 */
unsigned char get_text_attribute(unsigned char foreground, unsigned char background);

/*
 * Calculates the 16-bit value of a character and attribute.
 */
unsigned short get_attribute_text_value(unsigned char character, unsigned char attributes);
/*
 * Calculates the 16-bit value of a character, foreground and background.
 */
unsigned short get_text_value(unsigned char character, unsigned char foreground, unsigned char background);
/*
 * Clears the screen.
 */
void cls();
void swap_line(char up_down,short start_row,short end_row);
void scroll();
void putc(char c);
/*
 * This prints a string to the current console.
 */
void puts(const char *str);
unsigned char char_to_hex_char(unsigned char i);
void puthex(unsigned int i);
void putlhex(long long i);
void putdec(int num);
/*
 * Sends a hardware message to make the new cursor position blink.
 */
void update_cursor();
void move_curser(short y,short x);
/*
    This updates the current text color and background color of the console.
 */
void set_text_color(unsigned char foreground, unsigned char background);
void reset_text_color();
void init_video();
void println(const char * str);

void printf(const char *format, ...);
void printf_clr(char fg_color, const char *format, ...);
void printbin(int num);
void printlbin(long long num);
#endif /* __SCREEN_H */
