#include "kernel/types.h"
#include "user/user.h"
#include "user/threads.h"

#define NULL 0
int k = 0;
int k1 = 0;
void f3(void *arg)
{
    while (1)
    {
        k ++;
        // printf("plus 1 on k in f3\n");
    }
}

void f2(void *arg)
{
    while(1) {
         k ++;
         // printf("plus 1 on k in f2\n");
    }
}

void f1(void *arg)
{
    while(1) {
         
         if(k1 == 0)
         {
            k1++;
            // printf("plus 1 on k1 in f1 and yield\n");
            thread_yield();
         }
         else{
             k++;
             // printf("plus 1 on k1 in f1\n");
         }
    }
}

int main(int argc, char **argv)
{
    struct thread *t1 = thread_create(f1, NULL, 3);
    thread_add_runqueue(t1);
    struct thread *t2 = thread_create(f2, NULL, 3);
    thread_add_runqueue(t2);
    struct thread *t3 = thread_create(f3, NULL, 3);
    thread_add_runqueue(t3);

    thread_start_threading();
    printf("\nexited\n");
    exit(0);
}
