//  NibrasOS
//  kb.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-20T14:58:41+03:00.
//	Last modification time on 2017-07-29T23:53:35+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <kb.h>

static unsigned char shift_flag = 0;
static unsigned char arrow_flag = 0;

void keyboard_handler(struct regs *r){

	/* write EOI */
	write_port(0x20, 0x20);

	char keycode = read_port(KB_DATA_REG);

	/* left-shift behaviour setup */
	if(keycode == 0x2a || (unsigned char)keycode == 0xaa){
		if(!(keycode & 0x80)){
			shift_flag = !shift_flag;
		}
		else {
			shift_flag = !shift_flag;
		}
	}
	/* right-shift behaviour setup */
	else if(keycode == 0x36 || (unsigned char)keycode == 0xb6){
		if(!(keycode & 0x80)){
			shift_flag = !shift_flag;
		}
		else {
			shift_flag = !shift_flag;
		}
	}
	/* caps-lock behaviour setup */
	else if(keycode == 0x3A){
		shift_flag = !shift_flag;
	}
	else {
		if((unsigned char)keycode == 72){
			//up-arrow
			scroll_up();
		}
		else if((unsigned char)keycode == 80){
			//down-arrow
			scroll_down();
		}
		else if((unsigned char)keycode == 75){
			//left-arrow
			move_curser(0, -1);
		}
		else if((unsigned char)keycode == 77){
			//right-arrow
			move_curser(0, 1);
		}

}
	if(keycode < 0) return;
	if(!shift_flag)
		putc(st_keymap[(unsigned char)keycode]);
	else
		putc(st_shiftmap[(unsigned char)keycode]);
}
extern short set_kb_controller_repeate_delay(char delay_byte); //defined in start.asm
extern short test_PS2_controller(); //defined in start.asm
void keyboard_setup(){
	irq_install_handler(1,keyboard_handler);
	kb_enable();
	if(test_PS2_controller() != 0)
		printf_clr(RED,"ERROR! PS/2 Keyboard Controller couldn't be initialized\n");

//	if(set_kb_controller_repeate_delay(0x17) != 0){
//		set_text_color(RED,BLACK);
//		println("ERROR! PS/2 Keyboard Controller failed to set Repeate Delay");
//		reset_text_color();
//	}
}
//! read status from keyboard controller
unsigned char kb_read_status(){
	return inportb(KB_CTRL_REG);
}
void kb_send_cmd(unsigned char cmd) {
	while (1)	//wait for keyboard controller input buffer to be clear
		if ( (kb_read_status() & KB_STATUS_MASK_IN_BUF) == 0)
			break;
	write_port(KB_CTRL_REG, cmd);
}
void kb_encoder_send_cmd(unsigned char cmd){
	while (1)
		if ( (kb_read_status() & KB_STATUS_MASK_IN_BUF) == 0)
			break;
	//! send command byte to kybrd encoder
	outportb(0x60, cmd);
}
void kb_enable(){
	kb_send_cmd(0xAE); //0xAE: enable command
}
void kb_disable(){
	kb_send_cmd(0xAD); //0xAD: diable command
}

void kkybrd_set_leds(char num_led, char caps_led, char scroll_led) {
	unsigned char data = 0;
	//bit0 :enable scroll led
	//bit1 :enable num led
	//bit2 :enable caps led

	// set or clear the bit
	data = (scroll_led) ? (data | 0x1) : (data & 0x1);
	data = (num_led) ? (num_led | 0x2) : (num_led & 0x2);
	data = (caps_led) ? (num_led | 0x4) : (num_led & 0x4);

	kb_encoder_send_cmd(0xED);  //0xED: sets LEDs
	kb_encoder_send_cmd(data);
}
