#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#ifndef __riscv_atomic
#error "Your platform does not support atomic operations."
#endif

typedef struct {
	volatile int counter;
} atomic_t;

#define atomic_add(v, inc)	__sync_fetch_and_add(&((v)->counter), inc)
#define atomic_swap(v, swp)	__sync_lock_test_and_set(&((v)->counter), swp)
#define atomic_cas(v, cmp, swp)	__sync_val_compare_and_swap(&((v)->counter), cmp, swp)
#define atomic_inc(v)	atomic_add(v, 1)
#define atomic_dec(v)	atomic_add(v, -1)

#define atomic_read(v)	atomic_add(v, 0)
#define atomic_write(v,d)	atomic_swap(v, d)



#endif				/* __ATOMIC_H__ */
