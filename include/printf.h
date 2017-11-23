#ifndef __PRINTF_H
#define __PRINTF_H
#endif

#ifndef __VA_LIST_H
    #include <va_list.h>
#endif
#ifndef __TYPES_H
    #include <types.h>
#endif
#ifndef __STRING_H
    #include <string.h>
#endif
#ifndef __SCREEN_H
    #include <screen.h>
#endif

int is_format_letter(char c);
void vsprintf(char * str, void (*putchar)(char), const char * format, va_list arg);
void vsprintf_helper(char * str, void (*putchar)(char), const char * format, uint32_t * pos, va_list arg);
void printf(const char * format, ...);
void sprintf(char * buf, const char * fmt, ...);
