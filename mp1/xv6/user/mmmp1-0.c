#include "kernel/types.h"
#include "user/user.h"
#include "user/threads.h"

#define NULL 0

void f3(void *arg)
{
    int i = 10000;
    while (1) {
        printf("thread 3: %d\n", i++);
        if(i == 10005){
            thread_exit();
        }
        thread_yield();
    }
}
void f2(void *arg)
{
    int i = 0;
    while(1) {
        printf("thread 2: %d\n",i++);
        if (i == 10) {
            thread_exit();
        }
        thread_yield();
    }
}
void f1(void *arg)
{
    int i = 100;
    
    struct thread *t2 = thread_create(f2, NULL);
    thread_add_runqueue(t2);
    struct thread *t3 = thread_create(f3, NULL);
    thread_add_runqueue(t3);
    while(1) {
        printf("thread 1: %d\n", i++);
        if (i == 110) {
            thread_exit();
        }
        thread_yield();
    }
}
int main(int argc, char **argv)
{
    printf("mp1-0\n");
    struct thread *t1 = thread_create(f1, NULL);
    thread_add_runqueue(t1);
    thread_start_threading();
    printf("\nexited\n");
    exit(0);
}

/*

void f7(void *arg) {
    int i = 700;
    while (1) {
        printf("thread 7: %d\n", i++);
        if (i == 706) {
            thread_exit();
        }
        thread_yield();
    }
}

void f6(void *arg) {
    int i = 600;
    while (1) {
        printf("thread 6: %d\n", i++);
        if (i == 606) {
            thread_exit();
        }
        thread_yield();
    }
}

void f5(void *arg) {
    int i = 500;
    struct thread *t6 = thread_create(f6, NULL);
    thread_add_runqueue(t6);
    while (1) {
        printf("thread 5: %d\n", i++);
        if (i == 503) {
            thread_exit();
        }
        thread_yield();
    }
}

void f4(void *arg) {
    int i = 400;
    struct thread *t5 = thread_create(f5, NULL);
    thread_add_runqueue(t5);
    struct thread *t7 = thread_create(f7, NULL);
    thread_add_runqueue(t7);
    while (1) {
        printf("thread 4; %d\n", i++);
        if (i == 404) {
            thread_exit();
        }
        thread_yield();
    }
}

void f3(void *arg) {
    int i = 300;
    while (1) {
        printf("thread 3: %d\n", i++);
        if (i == 306) {
            thread_exit();
        }
        thread_yield();
    }
}

void f2(void *arg) {
    int i = 200;
    struct thread *t3 = thread_create(f3, NULL);
    thread_add_runqueue(t3);
    while (1) {
        printf("thread 2: %d\n", i++);
        if (i == 206) {
            thread_exit();
        }
        thread_yield();
    }
}

void f1(void *arg) {
    int i = 100;
    struct thread *t2 = thread_create(f2, NULL);
    thread_add_runqueue(t2);
    struct thread *t4 = thread_create(f4, NULL);
    thread_add_runqueue(t4);
    while (1) {
        printf("thread 1: %d\n", i++);
        if (i == 105) {
            thread_exit();
        }
        thread_yield();
    }
}

int main(int argc, char **argv) 
{
    printf("mp1-0\n");
    struct thread *t1 = thread_create(f1, NULL);
    thread_add_runqueue(t1);
    thread_start_threading();
    printf("\nexited\n");
    exit(0);
}

*/