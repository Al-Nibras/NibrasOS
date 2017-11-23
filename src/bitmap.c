//  NibrasOS
//  bitmap.c
//
//  Created by Abdulaziz Alfaifi <Al-Nibras> on 2017-06-22T17:15:23+03:00.
//	Last modification time on 2017-07-29T23:54:38+03:00.
//  Copyright © 2017 Abdulaziz Alfaifi. All rights reserved.
//

#include <bitmap.h>

inline void set_bit(unsigned int* bitmap,unsigned int bit){
	bitmap[INDEX_FROM_BIT(bit)] |= 1 << (OFFSET_FROM_BIT(bit));
}
inline void clear_bit(unsigned int* bitmap,unsigned int bit){
	bitmap[INDEX_FROM_BIT(bit)] &= ~(1 << (OFFSET_FROM_BIT(bit)));
}
inline unsigned int test_bit(unsigned int* bitmap, int bit){
	return ( bitmap[INDEX_FROM_BIT(bit)] & (1 << OFFSET_FROM_BIT(bit)) );
}
