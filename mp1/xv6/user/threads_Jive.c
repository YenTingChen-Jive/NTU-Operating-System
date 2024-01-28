#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0

static struct thread* current_thread = NULL;
static struct thread* root_thread = NULL;
static int id = 1;
static jmp_buf env_st;
static jmp_buf env_tmp;
int flag;                  // 設定env_tmp的setjmp, longjmp

struct thread *thread_create(void (*f)(void *), void *arg) {
    struct thread *t = (struct thread *) malloc(sizeof(struct thread));
    //unsigned long stack_p = 0;
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long) * 0x100);
    new_stack_p = new_stack + 0x100 * 8 - 0x2 * 8;
    t->fp = f;
    t->arg = arg;
    t->ID = id;
    t->buf_set = 0;
    t->stack = (void *) new_stack;
    t->stack_p = (void *) new_stack_p;
    id++;
    return t;
}

void thread_add_runqueue(struct thread *t) {
    if (current_thread == NULL) {     // 如果runqueue原本是空的，就將他設成root
        // printf("add root thread->ID = %d\n", t->ID);
        root_thread = t;
        t->parent = NULL;
        t->left = NULL;
        t->right = NULL;
        current_thread = t;
    }
    else {         // 如果runqueue不是空的，就判斷要加在：left, right, 或discard
        if (current_thread->left == NULL) {
            // printf("add left thread->ID = %d\n", t->ID);
            current_thread->left = t;
            t->parent = current_thread;
        }
        else if (current_thread->right == NULL) {
            // printf("add right thread->ID = %d\n", t->ID);
            current_thread->right = t;
            t->parent = current_thread;
        }
        else {
            // discard
            free(t->stack);
            free(t);
        }
    }
}

void thread_yield(void) {
    // save its context to the jmp_buf in struct thread using setjmp
    if (!setjmp(current_thread->env)) {
        schedule();
        dispatch();
    }
}

void dispatch(void) {
    if (current_thread->buf_set == 0) {       // 如果還沒setjmp，就先setjmp，並調整stack pointer
        current_thread->buf_set = 1;
        if (flag)
            longjmp(env_tmp, 1);
        setjmp(env_tmp);
        flag = 1;
        env_tmp->sp = (unsigned long)current_thread->stack_p;
        current_thread->fp(current_thread->arg);
    }
    else if (current_thread->buf_set == 1) {    // 如果已經setjmp，就用longjmp去找回原本的資訊就好
        longjmp(current_thread->env, 1);
    }
}

void schedule(void) {
    struct thread *next_thread;
    if (current_thread->left != NULL)                         // 有left child，就換left child
        next_thread = current_thread->left;
    else if (current_thread->right != NULL)                 // 有right child，就換right child
        next_thread = current_thread->right;
    else {                                      // 如果是leaf，就找preorder traversal裡的下一項
        struct thread *parent_thread = current_thread->parent;
        struct thread *cur_thread = current_thread;
        while (parent_thread != NULL && parent_thread->right == cur_thread) {
            cur_thread = cur_thread->parent;
            parent_thread = parent_thread->parent;
        }
        if (cur_thread == root_thread)
            next_thread = root_thread;
        else {
            if (parent_thread->right != NULL)
                next_thread = parent_thread->right;
            else if (parent_thread->right == NULL) {    ///////////////   可能有潛在bug？   ///////////////
                while (parent_thread == parent_thread->parent->right && parent_thread != root_thread) {
                    cur_thread = parent_thread;
                    parent_thread = parent_thread->parent;
                }
                if (parent_thread == root_thread)
                    next_thread = root_thread;
                else {
                    if (parent_thread->parent->right != NULL)
                        next_thread = parent_thread->parent->right;
                    else {
                        while (parent_thread->right == NULL && parent_thread != root_thread)
                            parent_thread = parent_thread->parent;
                        if (parent_thread == root_thread) {
                            if (root_thread->right == NULL)
                                next_thread = root_thread;
                            else 
                                next_thread = root_thread->right;
                        }
                        else 
                            next_thread = parent_thread->right;
                    }
                }
            } 
        }
    }
    current_thread = next_thread;
}

void thread_exit(void) {
    if (current_thread == root_thread && current_thread->left == NULL && current_thread->right == NULL) {    
        // 如果是最後一項，就free，然後longjmp回去
        free(current_thread->stack);
        free(current_thread);
        longjmp(env_st, 1);
    }
    else {
        if (current_thread->left == NULL && current_thread->right == NULL) {        
            // 如果是leaf，直接拿掉就好
            struct thread *cur_thread = current_thread;
            schedule();
            if (cur_thread->parent->left == cur_thread)
                cur_thread->parent->left = NULL;
            else if (cur_thread->parent->right == cur_thread)
                cur_thread->parent->right = NULL;
            free(cur_thread->stack);
            free(cur_thread);
            dispatch();
        }
        else {                               
            // 如果不是，那你很麻煩欸嗚嗚...而且你為什麼問題一堆又超難debug的啊?
            struct thread *cur_thread;
            cur_thread = current_thread;
            struct thread *last_thread;
            last_thread = cur_thread;
            while (last_thread->left != NULL || last_thread->right != NULL) {
                if (last_thread->right != NULL)
                    last_thread = last_thread->right;
                else
                    last_thread = last_thread->left;
            }
            if (last_thread->parent->left == last_thread)
                last_thread->parent->left = NULL;
            else if (last_thread->parent->right == last_thread)
                last_thread->parent->right = NULL;
            last_thread->parent = cur_thread->parent;
            last_thread->left = cur_thread->left;
            last_thread->right = cur_thread->right;
            if (cur_thread->left != NULL)
                cur_thread->left->parent = last_thread;
            if (cur_thread->right != NULL)
                cur_thread->right->parent = last_thread;
            if (cur_thread->parent != NULL) {
                if (cur_thread->parent->left == cur_thread)
                    cur_thread->parent->left = last_thread;
                else if (cur_thread->parent->right == cur_thread)
                    cur_thread->parent->right = last_thread;
            }
            if (cur_thread == root_thread)
                root_thread = last_thread;
            current_thread = last_thread;
            free(cur_thread->stack);
            free(cur_thread);
            /*printf("\t----------------------\n");
            printf("\tcurrent_thread = %d\n", current_thread->ID);
            printf("\tcurrent thread parent = %d, left = %d, right = %d\n", 
                    (current_thread->parent == NULL)? 0:current_thread->parent->ID,
                    (current_thread->left == NULL)? 0:current_thread->left->ID,
                    (current_thread->right == NULL)? 0:current_thread->right->ID);
            printf("\troot thread = %d\n", root_thread->ID);
            printf("\troot thread parent = %d, left = %d, right = %d\n", 
                    (root_thread->parent == NULL)? 0:root_thread->parent->ID,
                    (root_thread->left == NULL)? 0:root_thread->left->ID,
                    (root_thread->right == NULL)? 0:root_thread->right->ID);
            printf("\t----------------------\n");*/
            schedule();
            dispatch();
        }
    }
}

void thread_start_threading(void) {
    if (!setjmp(env_st)) {
        dispatch();
    }
}
