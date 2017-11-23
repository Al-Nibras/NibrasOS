//  NibrasOS
//  math.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-19T02:06:15+03:00.
//	Last modification time on 2017-07-29T23:52:55+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

/* power: num^p */
long long pow(int num, int p){
	long long number = num;
	for(int i = 1; i<=p;++i)
		number *= num;
	return number;
}
/* get power of number (number of digits) */
int pon(int num){
	int power = 0, temp = num;
	while (temp != 0) {
		++power;
		temp /= 10;
	}
	return power;
}

void swapi(int *x,int *y){
	int temp = *x;
	*x = *y;
	*y = temp;
}
void swapsh(short *x,short *y){
	short temp;
	temp = *y;
	*y   = *x;
	*x   = temp;
}
void swapc(char *x,char *y){
	char temp = *x;
	*x = *y;
	*y = temp;
}
