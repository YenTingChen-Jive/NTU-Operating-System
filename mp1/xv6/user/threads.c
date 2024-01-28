#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0

static struct thread *current_thread = NULL;
static struct thread *root_thread = NULL;
static int id = 1;
static jmp_buf env_st;
static jmp_buf env_tmp;

// TODO: necessary declares, if any
static struct thread *next_thread = NULL;
static struct thread *last_thread = NULL;
static int next = 0;
static int exited = 0;

void null_next(){
    next_thread = NULL;
    next = 0;
}

int check_null(struct thread *t){
    if(t -> left == NULL && t -> right != NULL) return 1;
    if(t -> right == NULL && t -> left != NULL) return 2;
    if(t -> right == NULL && t -> left == NULL) return 3;
    return 0;
}

void make_free(struct thread *t){
    free(t -> stack);
    free(t);
}

void change(struct thread *t1, struct thread *t2){
    t1-> left = t2 -> left;
    t1 -> right = t2-> right;
}

void given(struct thread *t1, struct thread *t2){
    t1 -> left = NULL;
    t1 -> right = NULL;
    t1 -> parent = t2;
}

void nextPreorder(struct thread *t){
    if (t->left)
        next_thread = t->left;

    else if (t->right)
          next_thread = t->right;

    else {
        struct thread *cur = t, *par = cur->parent;
        while (cur->parent != cur && par->right == cur) {
            cur = cur->parent;
            par = par->parent;
        }
    
        if (cur == par)
            next_thread = root_thread;
    
        else {
            if (par->right != NULL)
                next_thread = par->right;
            else {
                while (par == par->parent->right && par != root_thread) {
                    cur = par;
                    par = par->parent;
                }

                if (par == root_thread)
                    next_thread = root_thread;

                else {
                    if (par->parent->right != NULL)
                        next_thread = par->parent->right;

                    else {
                        while (par->right == NULL && par != root_thread)
                            par = par->parent;
                        if (par == root_thread) {
                            if (root_thread->right == NULL)
                                next_thread = root_thread;
                            else 
                                next_thread = root_thread->right;
                        }
                        else 
                            next_thread = par->right;
                    }
                }
            } 
        }
    }
    
}

void lastPreorder(struct thread *t){
    if (t == NULL) return;
    last_thread = t;
    lastPreorder(t -> left);
    lastPreorder(t -> right);
}


struct thread *thread_create(void (*f)(void *), void *arg)
{
    struct thread *t = (struct thread *)malloc(sizeof(struct thread));
    // unsigned long stack_p = 0;
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long)malloc(sizeof(unsigned long) * 0x100);
    new_stack_p = new_stack + 0x100 * 8 - 0x2 * 8;
    t->fp = f;
    t->arg = arg;
    t->ID = id;
    t->buf_set = 0;
    t->stack = (void *)new_stack;
    t->stack_p = (void *)new_stack_p;
    id++;
    return t;
}

void thread_add_runqueue(struct thread *t)
{
    if(current_thread == NULL){
        root_thread = t;
        current_thread = t;
        given(t, t);
    }
    else{
        given(t, current_thread);
        if (check_null(current_thread) == 1 || check_null(current_thread) == 3) current_thread -> left = t;
        else if (check_null(current_thread) == 2) current_thread -> right = t;
        else make_free(t);
    }
}
void thread_yield(void)
{
    if (!setjmp(current_thread -> env)){
        // current_thread -> buf_set = 1;
        schedule();
        dispatch();
    }
}

void dispatch(void){
    if(!(current_thread -> buf_set)){
        current_thread->buf_set = 1;
        if (exited)
            longjmp(env_tmp, 1);
        setjmp(env_tmp);
        exited = 1;
        env_tmp->sp = (unsigned long int)current_thread->stack_p;
        current_thread -> fp(current_thread -> arg);
    }
    else longjmp(current_thread -> env, 1);

    // thread_exit();
}

void schedule(void){
    null_next();
    nextPreorder(current_thread);
    if (next_thread == NULL) current_thread = root_thread;
    else current_thread = next_thread;
}

void thread_exit(void)
{
    if(check_null(current_thread) == 3 && current_thread == root_thread){
        make_free(current_thread);
        longjmp(env_st, 1);
        return;
        // Hint: No more thread to execute
    }

    if(check_null(current_thread) == 3){
        null_next();
        nextPreorder(current_thread);
        if (current_thread == current_thread -> parent -> left) current_thread -> parent -> left = NULL;
        if (next_thread == NULL) next_thread = root_thread;
        else if (current_thread == current_thread -> parent -> right) current_thread -> parent -> right = NULL;
        make_free(current_thread);
        current_thread = next_thread;
    }
    else if(current_thread == root_thread){
        lastPreorder(current_thread);
        if (last_thread == last_thread -> parent -> left) (last_thread -> parent) -> left = NULL;
        else if (last_thread == last_thread -> parent -> right) (last_thread -> parent) -> right = NULL;

        if (check_null(current_thread) == 2 || check_null(current_thread) == 0) current_thread -> left -> parent = last_thread;
        if (check_null(current_thread) == 1 || check_null(current_thread) == 0) current_thread -> right -> parent = last_thread;
        
        change(last_thread, current_thread);
        
        root_thread = last_thread;
        last_thread -> parent = last_thread;
        make_free(current_thread);

        current_thread = last_thread;
        schedule();
    }
    else
    {
        lastPreorder(current_thread);
        if (last_thread == last_thread -> parent -> left) last_thread -> parent -> left = NULL;
        else if (last_thread == last_thread -> parent -> right) last_thread -> parent -> right = NULL;

        if (current_thread == current_thread -> parent -> left) current_thread -> parent -> left = last_thread;
        else if (current_thread == current_thread->parent->right) current_thread->parent->right = last_thread;

        if (check_null(current_thread) == 2 || check_null(current_thread) == 0) current_thread->left->parent = last_thread;
        if (check_null(current_thread) == 1 || check_null(current_thread) == 0) current_thread->right->parent = last_thread;
        
        last_thread -> parent = current_thread -> parent;
        change(last_thread, current_thread);

        make_free(current_thread);
        current_thread = last_thread;
        schedule();
    }
    dispatch();
}

void thread_start_threading(void)
{
    if (!setjmp(env_st)) {
        dispatch();
    }
}