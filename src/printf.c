/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-09-08T15:42:18+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: printf.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:55:19+03:00
 */



#include <printf.h>

int is_format_letter(char c) {
    return c == 'c' ||  c == 'd' || c == 'i' ||c == 'e' ||c == 'E' ||c == 'f' ||c == 'g' ||c == 'G' ||c == 'o' ||c == 's' || c == 'u' || c == 'x' || c == 'X' || c == 'p' || c == 'n';
}

void vsprintf(char * str, void (*putchar)(char), const char * format, va_list arg) {
    uint32_t pos = 0;
    vsprintf_helper(str, putchar, format, &pos, arg);
}


void vsprintf_helper(char * str, void (*putchar)(char), const char * format, uint32_t * pos, va_list arg) {
    char c;
    int sign, ival, sys;
    char buf[512];
    char width_str[10];
    uint32_t uval;
    uint32_t size = 8;
    uint32_t i;
    int size_override = 0;
    memset(buf, 0, 512);

    while((c = *format++) != 0) {
        sign = 0;

        if(c == '%') {
            c = *format++;
            switch(c) {
                // Handle calls like printf("%08x", 0xaa);
                case '0':
                    size_override = 1;
                    // Get the number between 0 and (x/d/p...)
                    i = 0;
                    c = *format;
                    while(!is_format_letter(c)) {
                        width_str[i++] = c;
                        format++;
                        c = *format;
                    }
                    width_str[i] = 0;
                    format++;
                    // Convert to a number
                    size = atoi(width_str);
                case 'd':
                case 'u':
                case 'x':
                case 'p':
                    if(c == 'd' || c == 'u')
                        sys = 'd';
                    else
                        sys = 'x';

                    uval = ival = va_arg(arg, int);
                    if(c == 'd' && ival < 0) {
                        sign= 1;
                        uval = -ival;
                    }
                    itoa(buf,sys,uval);
                    uint32_t len = strlen(buf);
                    // If use did not specify width, then just use len = width
                    if(!size_override) size = len;
                    if((c == 'x' || c == 'p' || c == 'd') &&len < size) {
                        for(i = 0; i < len; i++) {
                            buf[size - 1 - i] = buf[len - 1 - i];
                        }
                        for(i = 0; i < size - len; i++) {
                            buf[i] = '0';
                        }
                    }
                    if(c == 'd' && sign) {
                        if(str) {
                            *(str + *pos) = '-';
                            *pos = *pos + 1;
                        }
                        else
                            (*putc)('-');
                    }
                    if(str) {
                        strcpy(str + *pos, buf);
                        *pos = *pos + strlen(buf);
                    }
                    else {
                        char * t = buf;
                        while(*t) {
                            putc(*t);
                            t++;
                        }
                    }
                    break;
                case 'c':
                    if(str) {
                        *(str + *pos) = (char)va_arg(arg, int);
                        *pos = *pos + 1;
                    }
                    else {
                        (*putc)((char)va_arg(arg, int));
                    }
                    break;
                case 's':
                    if(str) {
                        char * t = (char *) va_arg(arg, int);
                        strcpy(str + (*pos), t);
                        *pos = *pos + strlen(t);
                    }
                    else {
                        char * t = (char *) va_arg(arg, int);
                        while(*t) {
                            putc(*t);
                            t++;
                        }
                    }
                    break;
                default:
                    break;
            }
            continue;
        }
        if(str) {
            *(str + *pos) = c;
            *pos = *pos + 1;
        }
        else {
            (*putc)(c);
        }

    }
}

/*
 * Simplified version of printf and sprintf
 *
 * printf is sprintf is very similar, except that sprintf doesn't print to screen
 * */

void printf(const char * s, ...) {
    va_list ap;
    va_start(ap, s);
    vsprintf(0, putc, s, ap);
    va_end(ap);
}
void sprintf(char * buf, const char * fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, 0, fmt, ap);
    va_end(ap);
}
