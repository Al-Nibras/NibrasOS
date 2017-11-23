/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-04-19T11:59:17+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: io.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:53:00+03:00
 */



#include <io.h>

unsigned char inportb(unsigned short _port){
	unsigned char val;
	__asm__ __volatile__ ("inb %1, %0" : "=a" (val) : "dN" (_port));
	return val;
}
unsigned short inportw(unsigned short _port){
	unsigned short val;
	__asm__ __volatile__ ("inw %1, %0" : "=a" (val) : "dN" (_port));
	return val;
}
unsigned int inportl(unsigned short _port){
	unsigned int val;
	__asm__ __volatile__ ("inl %1, %0" : "=a" (val) : "dN" (_port));
	return val;
}
unsigned short inports(unsigned short _port) {
	unsigned short rv;
	__asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}
void inportsm(unsigned short port, unsigned char * data, unsigned long size) {
	__asm__ __volatile__ ("rep insw" : "+D" (data), "+c" (size) : "d" (port) : "memory");
}


void outportb (unsigned short _port, unsigned char val){
	__asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (val));
}
void outportw(unsigned short port,unsigned short val){
	__asm__ __volatile__ ("outw %1, %0" : : "dN" (port), "a" (val));
}
void outportl(unsigned short port,unsigned int val){
	__asm__ __volatile__ ("outl %1, %0" : : "dN" (port), "a" (val));
}
void outports(unsigned short _port, unsigned short _data) {
	__asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (_data));
}
void outportsm(unsigned short port, unsigned char * data, unsigned long size) {
	__asm__ __volatile__ ("rep outsw" : "+S" (data), "+c" (size) : "d" (port));
}
