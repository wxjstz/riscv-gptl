#ifndef __SMP_H__
#define __SMP_H__

#include <atomic.h>

typedef struct {
	atomic_t lock;
} spinlock_t;

static inline void spinlock_lock(spinlock_t *lock)
{
	while(atomic_swap(&lock->lock, 1)) {
        asm volatile ("nop");
        asm volatile ("nop");
	}
}

static inline void spinlock_unlock(spinlock_t *lock)
{
	atomic_write(&lock->lock, 0);
}

static inline void smp_wait(int hart_num)
{
	/* not always been inline-function,
	 * so need to reset count for next call */
	static atomic_t count;

	atomic_inc(&count); 
	while (atomic_read(&count) < hart_num) {
		/* waiting to enter */
        asm volatile ("nop");
        asm volatile ("nop");
	}

	atomic_dec(&count); 
	while (atomic_read(&count) > 0) {
		/* waitting to exit */
        asm volatile ("nop");
        asm volatile ("nop");
	}
}

#endif
