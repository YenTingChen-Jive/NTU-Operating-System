#ifndef THREADS_H_
#define THREADS_H_

// TODO: necessary includes, if any
#include "user/setjmp.h"
// TODO: necessary defines, if any

struct thread {
    void (*fp)(void *arg);
    void *arg;
    void *stack;
    void *stack_p;
    jmp_buf env;
    int ID;
    int buf_set;                // setjump buffer是否已經set好. 1: jmp_buf has been set; 0: jmp_buf hasn't been set.
    struct thread *left;
    struct thread *right;
    struct thread *parent;
};

struct thread *thread_create(void (*f)(void *), void *arg);
void thread_add_runqueue(struct thread *t);
void thread_yield(void);
void dispatch(void);
void schedule(void);
void thread_exit(void);
void thread_start_threading(void);

#endif // THREADS_H_
