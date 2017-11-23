//  NibrasOS
//  screen.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-15T19:42:31+03:00.
//	Last modification time on 2017-07-29T23:52:39+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <screen.h>

static unsigned char* get_latest_character_position();
static short get_latest_character_position_at_line(short y);
static void update_cursor_with_offset(unsigned short);


unsigned short *video_memory = (unsigned short *) VIDEO_MEMORY_ADDRESS;
unsigned char *u8video_memory = (unsigned char *) VIDEO_MEMORY_ADDRESS;
unsigned short* vram_up = 0;
unsigned short* vram_down = 0;
unsigned short* vram_up_start = 0;
unsigned short* vram_down_start = 0;

static unsigned SCREEN_ROWS = 25;
static unsigned SCREEN_COLUMNS = 80;

console main_console;


unsigned char get_text_attribute(unsigned char foreground, unsigned char background){
    return (background << 4) | (foreground & 0x0F);
}
unsigned short get_attribute_text_value(unsigned char character, unsigned char attributes){
    return character | (attributes << 8);
}
unsigned short get_text_value(unsigned char character, unsigned char foreground, unsigned char background){
    return character | (get_text_attribute(foreground, background) << 8);
}

void cls(){
    //clear current console options
    main_console.attributes = get_text_attribute(WHITE, BLACK);
    main_console.x = 0;
    main_console.y = 0;

    for(short *i=video_memory; i<0xbffff; ++i)
      *i = 0;
    update_cursor();
}

void swap_line(char up_down,short start_row,short end_row){
    if(up_down){
        for(short i = start_row; i<=end_row ;i++){
            unsigned char* first_line = 0xb8000 + (SCREEN_COLUMNS*2*i);
            unsigned char* next_line = first_line+(SCREEN_COLUMNS*2);

            for(int j = 0; j<SCREEN_COLUMNS*2; ++j)
                *first_line++ = *next_line++;
        }
    }
    else {
        for(short i = start_row; i>=end_row ;i--){
            unsigned char* first_line = 0xb8000 + (SCREEN_COLUMNS*2*i);
            unsigned char* next_line = first_line-(SCREEN_COLUMNS*2);

            for(int j = 0; j<SCREEN_COLUMNS*2; ++j)
                *first_line++ = *next_line++;
        }
    }
}

void scroll(){
    if (main_console.y >= SCREEN_ROWS){
        if(vram_up){
            memcpy((char*)vram_up,VIDEO_MEMORY_ADDRESS,SCREEN_COLUMNS*2);
            vram_up+=SCREEN_COLUMNS;
        }
        //swapping lines
        swap_line(1,0, SCREEN_ROWS-1);
        --main_console.y;
    }
}
void scroll_down(){
    if(vram_down>vram_down_start){ //if there's a line below this page
        memcpy((char*)vram_up,VIDEO_MEMORY_ADDRESS,SCREEN_COLUMNS*2);
        vram_up+=SCREEN_COLUMNS;
        memsetw(VIDEO_MEMORY_ADDRESS,0,SCREEN_COLUMNS);
        swap_line(1,0,SCREEN_ROWS-1);

        vram_down-=SCREEN_COLUMNS;
        memcpy((char*)(VIDEO_MEMORY_ADDRESS+( (SCREEN_ROWS-1)*SCREEN_COLUMNS*2)),(char*)vram_down,SCREEN_COLUMNS*2);
     }
}
void scroll_up(){
    if(vram_up>vram_up_start){ //if there's a line above this page
        memcpy((char*)vram_down,VIDEO_MEMORY_ADDRESS+((SCREEN_ROWS-1)*SCREEN_COLUMNS*2),SCREEN_COLUMNS*2);
        vram_down+=SCREEN_COLUMNS;
        memsetw((char*)(VIDEO_MEMORY_ADDRESS+( (SCREEN_ROWS-1)*SCREEN_COLUMNS*2)),0,SCREEN_COLUMNS);
        swap_line(0,SCREEN_ROWS, 0);
        vram_up-=SCREEN_COLUMNS;
        memcpy(VIDEO_MEMORY_ADDRESS,(char*)vram_up,SCREEN_COLUMNS*2);
     }
}

void putc(char c){
    unsigned short * cellPos;

    // Backspace - move cursor back one position.
    if (c == 0x08) /* '\b' */ {
        if(main_console.x <= 0 && main_console.y <= 0) return;

        if (main_console.x > 0){
            cellPos = video_memory + ((main_console.y * SCREEN_COLUMNS) + main_console.x) -1;
            *cellPos = get_attribute_text_value('\0', main_console.attributes);

            do {
                main_console.x--;
                cellPos-=2;
            }while(*cellPos == ' ' && *(cellPos-2) == ' ' && main_console.x > 0);
        }
        else {
            cellPos = video_memory + ((main_console.y * SCREEN_COLUMNS) + main_console.x) -1;
            *cellPos = get_attribute_text_value('\0', main_console.attributes);
            --main_console.y;

            unsigned char* offset = get_latest_character_position();
            short x = (short)offset - ((short)video_memory + ((main_console.y * SCREEN_COLUMNS*2) + 0));
            x /= 2;
            main_console.x = x;
        }
    }
    // Tab - move cursor towards next x which is divisible by 8
    else if (c == 0x09) {
        main_console.x = (main_console.x + 8) & ~(8 - 1);
    }
    // Carriage return - set x to 0
    else if (c == '\r'){
        main_console.x = 0;
    }
    // New line
    else if (c == '\n'){
        main_console.x = 0;
        main_console.y++;
    }
    // Printable character
    else if (c >= ' '){
        cellPos = video_memory + ((main_console.y * SCREEN_COLUMNS) + main_console.x);
        *cellPos = get_attribute_text_value(c, main_console.attributes);
        main_console.x++;
    }

    // Move line down y 1 if we have reached the edge of the columns
    if (main_console.x > SCREEN_COLUMNS){
        main_console.x = 0;
        main_console.y++;
    }

    // Scroll and update cursor.
    scroll();
    update_cursor();
}

void puts(const char *str){
    for (unsigned int i = 0; i < strlen(str); i++)
        putc(str[i]);
}

unsigned char char_to_hex_char(unsigned char i){
    if (i < 10){
        return '0' + i;
    } else {
        return 'A' + (i - 10);
    }
}

void puthex(unsigned int i){
    unsigned char result[11]; // 11 for 0x + 8 characters + '\0'
    result[0] = '0';
    result[1] = 'x';
    result[10] = 0;

    int pos = 8;
    unsigned char current;

    while (pos > 0){
        current = (unsigned char) i & 0x0f;
        result[1 + pos] = (unsigned char) char_to_hex_char(current);
        i = i >> 4;
        pos--;
    }
    puts(result);
}
void putlhex(long long i){
    unsigned char result[19]; // 11 for 0x + 8 characters + '\0'
    result[0] = '0';
    result[1] = 'x';
    result[19] = '\0';

    unsigned int pos = 16;
    unsigned char current;

    while (pos > 0){
        current = (unsigned char) i & 0x0f;
        result[1 + pos] = (unsigned char) char_to_hex_char(current);
        i = i >> 4;
        pos--;
    }

    puts(result);
}

void putdec(int num){
    if(num < 0){
        putc('-');
        num *= -1;
    }
    if(num < 10)
        putc(num+'0');
    else {
        int power = pon(num);
        char str[power+1]; str[power] = 0;

        while(num > 0){
            str[--power] = (num % 10)+'0';
            num /= 10;
        }
        puts(str);
    }
}

/*
 * Sends a hardware message to make the new cursor position blink.
 */
void update_cursor(){
    unsigned short offset = (main_console.y * SCREEN_COLUMNS) + main_console.x;

    outportb(0x3D4, 14);
    outportb(0x3D5, (offset >> 8)&0xff); //offset's high-bits
    outportb(0x3D4, 15);
    outportb(0x3D5, offset&0xff); //offset's low-bits
}
void move_curser(short y, short x){
    if(main_console.x+x<0){
        if( (main_console.y-1) < 0){
             scroll_up();
             main_console.x = get_latest_character_position_at_line(main_console.y);
             update_cursor();
             return;
         }
        unsigned short x = get_latest_character_position_at_line(main_console.y-1);
        while(x++<0)
            main_console.y--;

        main_console.x = x;
        main_console.y--;
        update_cursor();
        return;
     }
    else if( (main_console.x+x)>get_latest_character_position_at_line(main_console.y)){
        if(main_console.y+1 > SCREEN_ROWS) scroll_down();
        else main_console.y++;
        main_console.x = 0;
        update_cursor();
        return;
    }
    else if(main_console.y+y>SCREEN_ROWS) return;
    else {
        main_console.x+=x;
        main_console.y+=y;
        update_cursor();
    }
}
static void update_cursor_with_offset(unsigned short offset){
    outportb(0x3D4, 14);
    outportb(0x3D5, offset << 8);
    outportb(0x3D4, 15);
    outportb(0x3D5, offset);
}
/*
 * This updates the current text color and background color of the console.
 */
void set_text_color(unsigned char foreground, unsigned char background){
    main_console.attributes = get_text_attribute(foreground, background);
}
void reset_text_color(){
    main_console.attributes = get_text_attribute(WHITE, BLACK);
}

static unsigned char* get_latest_character_position(){
    short x = main_console.x, y = main_console.y;
    char* pos = video_memory + ((y * SCREEN_COLUMNS) + x);
    while (*pos != '\0'){
        pos+=2;
        if(main_console.x <=0){
            main_console.x = 0;
            --y;
        }
    }
    return pos;
}
static short get_latest_character_position_at_line(short y){
    short x = 0;
    char* pos = video_memory + ((y * SCREEN_COLUMNS) + x);
    while (*pos != '\0' && *pos != '\n'){
        pos+=2;
        x+=1;
    }
    if(*pos != '\0')
        return x;
    else
        return x-1;
}

void init_video(int mode){
    if(mode == 1){
      SCREEN_ROWS = 50; SCREEN_COLUMNS = 80;
    }
    else if(mode == 2){
      SCREEN_ROWS = 60; SCREEN_COLUMNS = 90;
    }
    else{
        SCREEN_ROWS = 25; SCREEN_COLUMNS = 80;
    }
    set_text_mode(mode);
    cls();
}
void println(const char * str){
    if(str)
        puts(str);
    puts("\n");
}

void printf_clr(char fg_color, const char *format, ...){
    set_text_color(fg_color, BLACK);
    va_list ap;
    va_start(ap, format);
    vsprintf(0, putc, format, ap);
    va_end(ap);
    reset_text_color();
}

void printbin(int num){
    if(!num) return;
    for(int i = 31; i>=0; --i)
        printf("%d",(num & (1<<i)) ? 1 : 0);
}
void printlbin(long long num){
    if(!num) return;
    for(int i = 63; i>=0; --i)
        printf("%d",(num & (1<<i)) ? 1 : 0);
}
