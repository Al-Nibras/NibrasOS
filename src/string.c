//  NibrasOS
//  string.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-20T20:06:58+03:00.
//	Last modification time on 2017-07-29T23:48:36+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <string.h>

char small_to_cap(char c){
	if(c>=97 && c<=122)
		return c-32;
	return c;
}
char cap_to_small(char c){
	if(c>=65 && c<=90)
		return c+32;
	return c;
}

void itoa(char *buf, int base, int d)
{
	 char *p = buf;
	 char *p1, *p2;
	 unsigned long ud = d;
	 int divisor = 10;

	if(base == 'b'){
		for(int i = 0; i<32; ++i)
	        buf[i] = (d & (1<<i)) ? '1' : '0';
		return;
	}
	 /* If %d is specified and D is minus, put `-' in the head. */
	else if (base == 'd' && d < 0)
	{
		  *p++ = '-';
		  buf++;
		  ud = -d;
	}
	else if (base == 'x')
		divisor = 16;

	 /* Divide UD by DIVISOR until UD == 0. */
	do{
		  int remainder = ud % divisor;
		  *p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
	}while (ud /= divisor);

	 /* Terminate BUF. */
	*p = 0;

	 /* Reverse BUF. */
	p1 = buf;
	p2 = p-1;
	while (p1 < p2)
	{
		  char tmp = *p1;
		  *p1 = *p2;
		  *p2 = tmp;
		  p1++;
		  p2--;
	}
}
unsigned long long strlen(const char* str){
	unsigned long long counter = 0;
	while(*str++ && counter <= MAX_ULONG) //avoiding overflow
		++counter;
	return counter;
}
//safe strcopy
void strcpy_s(char* dest,char* src){
	unsigned long long counter = strlen(src), i;
	/*NOTE:	a good programmer would use his own loop to copy >`MAX_ULONG` chars if needed,
			like this: for(very_large_type i  = 0; *src ; i += MAX_ULONG) {dest[i] = src[i];} */
	for(i = 0;i<counter; ++i)
		dest[i] = src[i];
}
int strcpy(char* dest,char* src){
	int counter = 0;
	while (*src){
		 *dest++ = *src++;
		 ++counter;
	 }
	 if(!(*src))
	 	*dest = 0; 	//NULL-terminated string
	 return counter;
}
int strcmp(const char* a, const char* b){
	if(!a || !b) return -1;
	int flag = 0;
	size_t a_len = strlen(a);
	size_t b_len = strlen(b);

	for(size_t i = 0; i<a_len && i<b_len; ++i){
		if(a[i] != b[i]){
			 flag = 1;
			  break;
		}
	}
	return flag;
}
int strncmp(const char* s1, const char* s2, size_t n)
{
    while(n--)
        if(*s1++!=*s2++)
            return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
    return 0;
}
// char * strdup(const char *str) {
// 	int len = strlen(str);
// 	char * out = malloc(sizeof(char) * (len+1));
// 	memcpy(out, str, len+1);
// 	return out;
// }
char * strtok_r(char * str, const char * delim, char ** saveptr) {
	char * token;
	if (!str)
		str = *saveptr;
	str += strspn(str, delim);
	if (*str == '\0') {
		*saveptr = str;
		return 0;
	}
	token = str;
	str = strpbrk(token, delim);
	if (!str) {
		*saveptr = (char *)lfind(token, '\0');
	} else {
		*str = '\0';
		*saveptr = str + 1;
	}
	return token;
}

unsigned int lfind(const char * str, const char accept) {
	unsigned int i = 0;
	while ( str[i] != accept) {
		i++;
	}
	return (unsigned int)(str) + i;
}

unsigned int rfind(const char * str, const char accept) {
	unsigned int i = strlen(str) - 1;
	while (str[i] != accept) {
		if (i == 0) return UINT32_MAX;
		i--;
	}
	return (unsigned int)(str) + i;
}

char * strstr(const char * haystack, const char * needle) {
	const char * out = 0;
	const char * ptr;
	const char * acc;
	const char * p;
	unsigned int s = strlen(needle);
	for (ptr = haystack; *ptr != '\0'; ++ptr) {
		unsigned int accept = 0;
		out = ptr;
		p = ptr;
		for (acc = needle; (*acc != '\0') && (*p != '\0'); ++acc) {
			if (*p == *acc) {
				accept++;
				p++;
			} else {
				break;
			}
		}
		if (accept == s) {
			return (char *)out;
		}
	}
	return 0;
}

unsigned char startswith(const char * str, const char * accept) {
	unsigned int s = strlen(accept);
	for (unsigned int i = 0; i < s; ++i) {
		if (*str != *accept) return 0;
		str++;
		accept++;
	}
	return 1;
}

unsigned int strspn(const char * str, const char * accept) {
	const char * ptr = str;
	const char * acc;

	while (*str) {
		for (acc = accept; *acc; ++acc) {
			if (*str == *acc) {
				break;
			}
		}
		if (*acc == '\0') {
			break;
		}

		str++;
	}

	return str - ptr;
}

char * strpbrk(const char * str, const char * accept) {
	const char *acc = accept;

	if (!*str) {
		return 0;
	}

	while (*str) {
		for (acc = accept; *acc; ++acc) {
			if (*str == *acc) {
				break;
			}
		}
		if (*acc) {
			break;
		}
		++str;
	}

	if (*acc == '\0') {
		return 0;
	}

	return (char *)str;
}

char *strncpy(char *destString, const char *sourceString,int maxLength)
{
    unsigned count;

    if ((destString == (char *) NULL) || (sourceString == (char *) NULL))
    {
        return (destString = NULL);
    }

    if (maxLength > 255)
        maxLength = 255;

    for (count = 0; (int)count < (int)maxLength; count ++)
    {
        destString[count] = sourceString[count];

        if (sourceString[count] == '\0')
            break;
    }

    if (count >= 255)
    {
        return (destString = NULL);
    }

    return (destString);
}


void strcat(void *dest,const void *src)
{
    char * end = (char*)dest + strlen(dest);
    memcpy((char*)end,(char*)src,strlen((char*)src));
    end = end + strlen((char*)src);
    *end = '\0';
}

char * strdup(const char * src) {
    int len = strlen(src) + 1;
    char * dst = kmalloc(len);
    memcpy(dst, src, len);
    return dst;
}

char *strsep(char **stringp, const char *delim) {
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;
    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
}

/*
   Split a string into list of strings
   */
list_t * str_split(const char * str, const char * delim, unsigned int * numtokens) {
    list_t * ret_list = list_create();
    char *s = strdup(str);
    char *token, *rest = s;
    while ((token = strsep(&rest, delim)) != NULL) {
        if(!strcmp(token, ".")) continue;
        if(!strcmp(token, "..")) {
            if(list_size(ret_list) > 0) list_pop(ret_list);
            continue;
        }
        list_push(ret_list, strdup(token));
        if(numtokens) (*numtokens)++;
    }
    kfree(s);
    return ret_list;
}
/*
 * Reconstruct the string with tokens and delimiters
 * */
char * list2str(list_t * list, const char * delim) {
    char * ret = kmalloc(256);
    memset(ret, 0, 256);
    int len = 0, ret_len = 256;
    while(list_size(list)> 0) {
        char * temp = list_pop(list)->val;
        int len_temp = strlen(temp);
        if(len + len_temp + 1 + 1 > ret_len) {
            ret_len = ret_len * 2;
			kfree(ret);
            ret = kmalloc(ret_len);
            len = len + len_temp + 1;
        }
        strcat(ret, delim);
        strcat(ret, temp);
    }
    return ret;
}

void str_to_small(char* str){
	if(!str) return;
	size_t str_size = strlen(str);
	for(size_t i = 0; i<str_size;++i)
		str[i] = cap_to_small(str[i]);
}
void str_to_cap(char* str){
	if(!str) return;
	size_t str_size = strlen(str);
	for(size_t i = 0; i<str_size;++i)
		str[i] = small_to_cap(str[i]);
}
int isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}
int atoi(char * string) {
    int result = 0;
    unsigned int digit;
    int sign;

    while (isspace(*string)) {
        string += 1;
    }

    /*
     * Check for a sign.
     */

    if (*string == '-') {
        sign = 1;
        string += 1;
    } else {
        sign = 0;
        if (*string == '+') {
            string += 1;
        }
    }

    for ( ; ; string += 1) {
        digit = *string - '0';
        if (digit > 9) {
            break;
        }
        result = (10*result) + digit;
    }

    if (sign) {
        return -result;
    }
    return result;
}
