/**
 * @Author: Abdulaziz Alfaifi <al-nibras>
 * @Date:   2017-08-27T14:14:24+03:00
 * @Email:  al-nibras@outlook.sa
 * @Project: NibrasOS
 * @Filename: spinlock.c
 * @Last modified by:   al-nibras
 * @Last modified time: 2017-11-23T22:57:04+03:00
 */



#ifndef __SPINLOCK_H
#define __SPINLOCK_H
#endif

static unsigned int volatile spin_lock = 0;
extern void sys_acquire(unsigned int* lock_ptr); //defined in start.asm
extern void sys_release(unsigned int* lock_ptr); //defined in start.asm

void lock(unsigned int *semaphore){
    if(!semaphore) return;
    sys_acquire(semaphore);
}
void unlock(unsigned int *semaphore){
    if(!semaphore) return;
    sys_release(semaphore);
}
void spinlock(){
    lock(&spin_lock);
}
void spinunlock(){
    unlock(&spin_lock);
}
