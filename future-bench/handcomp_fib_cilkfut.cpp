#include "../src/future.h"
#include <cilk/cilk.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "ktiming.h"
#include "internal/abi.h"
#include "../cilkrtssuspend/runtime/cilk_fiber.h"
#include "../cilkrtssuspend/runtime/os.h"
#include "../cilkrtssuspend/runtime/jmpbuf.h"
#include "../cilkrtssuspend/runtime/global_state.h"
#include "../cilkrtssuspend/runtime/full_frame.h"
#include "../cilkrtssuspend/runtime/scheduler.h"
#include "../cilkrtssuspend/runtime/local_state.h"

#ifndef TIMES_TO_RUN
#define TIMES_TO_RUN 1 
#endif

//#define TEST_INTEROP_PRE_FUTURE_CREATE
//#define TEST_INTEROP_POST_FUTURE_CREATE
//#define TEST_INTEROP_MULTI_FUTURE

//#define FUTURE_AFTER_SYNC

/* 
 * fib 39: 63245986
 * fib 40: 102334155
 * fib 41: 165580141 
 * fib 42: 267914296
 */

extern CILK_ABI_VOID __cilkrts_leave_future_frame(__cilkrts_stack_frame *sf);
extern CILK_ABI_VOID __cilkrts_switch_fibers_back(cilk_fiber* curr_fiber, cilk_fiber* new_fiber);
extern CILK_ABI(cilk_fiber*) __cilkrts_switch_fibers();

extern "C" {
extern CILK_ABI_VOID __cilkrts_detach(struct __cilkrts_stack_frame *sf);
extern CILK_ABI_VOID __cilkrts_pop_frame(struct __cilkrts_stack_frame *sf);
extern CILK_ABI_VOID __cilkrts_enter_frame_1(__cilkrts_stack_frame *sf);

void** cilk_fiber_get_resume_jmpbuf(cilk_fiber *self);
void cilk_fiber_do_post_switch_actions(cilk_fiber *self);
char* cilk_fiber_get_stack_base(cilk_fiber *self);
}

int fib(int n);
void fib_helper(int *res, int n);

#define ALIGN_MASK  (~((uintptr_t)0xFF))
static char* __attribute__((always_inline)) get_sp_for_fiber(char* stack_base) {
    // Make the stack pointer 256-byte aligned
    char* new_stack_base = stack_base - 256;
    new_stack_base = (char*)((size_t)new_stack_base & ALIGN_MASK);
    return new_stack_base;
}

void  __attribute__((noinline)) fib_fut(cilk::future<int> *x, int n) {
    __cilkrts_stack_frame* sf = (__cilkrts_stack_frame*) alloca(sizeof(__cilkrts_stack_frame));;
    __cilkrts_enter_frame_fast_1(sf);
    __cilkrts_detach(sf);
    
    void *__cilk_deque = x->put(fib(n));
    if (__cilk_deque) {
        __cilkrts_resume_suspended(__cilk_deque, 1);
    }

    __cilkrts_pop_frame(sf);
    __cilkrts_leave_future_frame(sf);
}

int  __attribute__((noinline)) fib(int n) {
    int x;
    int y;

    if(n < 2) {
        return n;
    }

    __cilkrts_stack_frame sf;
    __cilkrts_enter_frame_1(&sf);

    #ifdef TEST_INTEROP_PRE_FUTURE_CREATE
        #pragma message ("using spawn pre fut fib interop")
        if (!CILK_SETJMP(sf.ctx)) {
            fib_helper(&y, n-2);
        }
    #endif

    cilk::future<int> x_fut = cilk::future<int>();

    sf.flags |= CILK_FRAME_FUTURE_PARENT;

    // TODO: Is there an easier/better way to do this? Doing this avoids
    //       locking the frame when we run out of fibers on the future fiber deque.
    cilk_fiber *volatile initial_fiber = cilk_fiber_get_current_fiber();
     
    if (!CILK_SETJMP(cilk_fiber_get_resume_jmpbuf(initial_fiber))) {

        char *volatile old_sp = NULL;
        __asm__ volatile ("mov %%rsp,%0"
                          : "=r" (old_sp));

        char* new_sp = NULL;

        // TODO: Why does writing it this way result in significant performance gains!?
        cilk_fiber *volatile fut_fiber = NULL;
        fut_fiber = __cilkrts_switch_fibers();
        new_sp = get_sp_for_fiber(cilk_fiber_get_stack_base(fut_fiber));

        __asm__ volatile ("mov %0,%%rsp"
                          : : "r" (new_sp));

        fib_fut(&x_fut, n-1);

        // Move our stack back!
        __asm__ volatile ("mov %0,%%rsp"
                          : : "r" (old_sp));

        __cilkrts_switch_fibers_back(fut_fiber, initial_fiber);
    }
    cilk_fiber_do_post_switch_actions(initial_fiber);
    sf.flags &= ~(CILK_FRAME_FUTURE_PARENT);

    #ifdef TEST_INTEROP_POST_FUTURE_CREATE
        #pragma message ("using using spawn post fut fib interop")
        if (!CILK_SETJMP(sf.ctx)) {
            fib_helper(&y, n-2);
        }
    #elif defined(TEST_INTEROP_MULTI_FUTURE)
        #pragma message ("using future fib interop")
        cilk::future<int> y_fut = cilk::future<int>();

        sf.flags |= CILK_FRAME_FUTURE_PARENT;

        initial_fiber = cilk_fiber_get_current_fiber();

        if (!CILK_SETJMP(cilk_fiber_get_resume_jmpbuf(initial_fiber))) {
            cilk_fiber *fut_fiber = __cilkrts_switch_fibers();

            volatile char* old_sp = NULL;
            __asm__ volatile ("mov %%rsp,%0"
                              : "=r" (old_sp));

            char* new_sp = get_sp_for_fiber(cilk_fiber_get_stack_base(fut_fiber));

            __asm__ volatile ("mov %0,%%rsp"
                              : : "r" (new_sp));

            fib_fut(&y_fut, n-2);

            // Move our stack back!
            __asm__ volatile ("mov %0,%%rsp"
                              : : "r" (old_sp));

            __cilkrts_switch_fibers_back(fut_fiber, initial_fiber);
        }
        cilk_fiber_do_post_switch_actions(initial_fiber);
        sf.flags &= ~(CILK_FRAME_FUTURE_PARENT);
    
        y = y_fut.get();
    #elif !defined(TEST_INTEROP_PRE_FUTURE_CREATE)
        #pragma message ("using regular fib (no interop)")
        y = fib(n-2);
    #endif

    #ifndef FUTURE_AFTER_SYNC
        #pragma message ("future get before sync")
        x = x_fut.get();
    #endif

    #if defined(TEST_INTEROP_PRE_FUTURE_CREATE) || defined(TEST_INTEROP_POST_FUTURE_CREATE)
        #pragma message ("Added a synch to the function")

        if (sf.flags & CILK_FRAME_UNSYNCHED) {
            if (!CILK_SETJMP(sf.ctx)) {
                __cilkrts_sync(&sf);
            }
        }
    #endif

    #ifdef FUTURE_AFTER_SYNC
        #pragma message ("future get after sync")
        x = x_fut.get();
    #endif

    int _tmp = x+y;

    // If we aren't careful, it turns out lto
    // gets too agressive and starts popping
    // frames at inappropriate moments
    __asm__ volatile ("" ::: "memory");
    __cilkrts_pop_frame(&sf);
    __cilkrts_leave_frame(&sf);

    return _tmp;
}

void __attribute__((noinline)) fib_helper(int* res, int n) {
    __cilkrts_stack_frame* sf = (__cilkrts_stack_frame*) alloca(sizeof(__cilkrts_stack_frame));;
    __cilkrts_enter_frame_fast_1(sf);
    __cilkrts_detach(sf);

    *res = fib(n);
    
    __cilkrts_pop_frame(sf);
    __cilkrts_leave_frame(sf);
}

int __attribute__((noinline)) run(int n, uint64_t *running_time) {
    __cilkrts_stack_frame* sf = (__cilkrts_stack_frame*) alloca(sizeof(__cilkrts_stack_frame));;
    __cilkrts_enter_frame_1(sf);

    int res;
    clockmark_t begin, end; 

    for(int i = 0; i < TIMES_TO_RUN; i++) {
        begin = ktiming_getmark();

        if (!CILK_SETJMP(sf->ctx)) {
            fib_helper(&res, n);
        }

        if (sf->flags & CILK_FRAME_UNSYNCHED) {
            if (!CILK_SETJMP(sf->ctx)) {
                __cilkrts_sync(sf);
            }
        }

        end = ktiming_getmark();
        running_time[i] = ktiming_diff_usec(&begin, &end);
    }


    __cilkrts_pop_frame(sf);
    __cilkrts_leave_frame(sf);

    return res;
}

int main(int argc, char * args[]) {
    int n;
    uint64_t running_time[TIMES_TO_RUN];

    if(argc != 2) {
        fprintf(stderr, "Usage: fib [<cilk-options>] <n>\n");
        exit(1);
    }
    
    n = atoi(args[1]);

    int res = run(n, &running_time[0]);
    cilkg_set_param("local stacks", "256");

    printf("Res: %d\n", res);

    if( TIMES_TO_RUN > 10 ) 
        print_runtime_summary(running_time, TIMES_TO_RUN); 
    else 
        print_runtime(running_time, TIMES_TO_RUN); 

    return 0;
}

