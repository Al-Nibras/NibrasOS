#ifndef __BITMAP_H
#define __BITMAP_h

#define SIZE 32							//change when required
#define INDEX_FROM_BIT(b) (b/SIZE)
#define OFFSET_FROM_BIT(b) (b%SIZE)


inline void set_bit(unsigned int* bitmap,unsigned int bit);
inline void clear_bit(unsigned int* bitmap,unsigned int bit);
inline unsigned int test_bit(unsigned int* bitmap, int bit);


#endif /* __BITMAP_H */
