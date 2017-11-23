//  NibrasOS
//  timer.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-19T21:19:19+03:00.
//	Last modification time on 2017-07-29T23:51:41+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <system.h>

/* 1.193182 MHz */
#define DIVIDER 1193182

extern void read_PIT_count();

static volatile unsigned int ticks = 0; //must be unsigned: when it overflows, it will reset to 0
extern char tasking_enabled; //defined int process.c

static void timer_handler(struct regs *r)
{
	if(!tasking_enabled) return;
	
	ticks++;
	if (ticks % 8 == 0){
		/* Every 18 clocks (approximately 1 second passed) */
		//printf("ticks: %d\tOne second has passed\n",ticks);
		sched(r);
	}
}

void init_timer(unsigned int hz)
{
	// Firstly, register our timer callback.
	irq_install_handler(0, timer_handler);

	/* The value we send to the PIT is the value to divide it's input clock
	   (1193180 Hz) by, to get our required frequency. Important to note is
	   that the divisor must be small enough to fit into 16-bits. */

	unsigned int divisor = DIVIDER / hz;	 /* Calculate our divisor , the divisor must be 16 bits */
	/*
		16-bits: 0 up to 65535, this HW counter starts at 1 and its zero value is considered 65536
		max = 1193182 / 1 = 1193182 hz	   (least frequent)
		min = 1193182 / 65536 ≈ 18.2065 hz (most frequent)
	*/
	outportb(0x43, 0x36);             /* tell the PIT which channel we're setting: set channel 0x43, command channel*/
	outportb(0x40, (unsigned char)divisor & 0xff);   /* send low byte */
	outportb(0x40, (unsigned char)(divisor >> 8) & 0xff);     /* send high byte */
}

void delay(unsigned int hz){ /* delay(1000) approximately is 10 seconds when init_timer(1); parameterized with 1 */
	if(ticks>0){
		unsigned int period = ticks+hz;
		while (ticks<period); /* simply do nothing */
	}
}
void sleep(unsigned int sec){
	delay(sec*100);
}
