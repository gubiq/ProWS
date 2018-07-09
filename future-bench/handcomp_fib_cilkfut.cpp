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

#ifndef TIMES_TO_RUN
#define TIMES_TO_RUN 5 
#endif

/* 
 * fib 39: 63245986
 * fib 40: 102334155
 * fib 41: 165580141 
 * fib 42: 267914296
 */

extern int ZERO;

extern CILK_ABI_VOID __cilkrts_future_sync(__cilkrts_stack_frame *sf);
extern CILK_ABI_VOID __cilkrts_leave_future_frame(__cilkrts_stack_frame *sf);
extern CILK_ABI_VOID __cilkrts_leave_future_parent_frame(__cilkrts_stack_frame *sf);
extern CILK_ABI_VOID __cilkrts_switch_fibers_back(__cilkrts_stack_frame* first_frame, cilk_fiber* curr_fiber, cilk_fiber* new_fiber);
extern CILK_ABI_VOID __cilkrts_switch_fibers(__cilkrts_stack_frame* first_frame);

extern "C" {
extern CILK_ABI_VOID __cilkrts_detach(struct __cilkrts_stack_frame *sf);
extern CILK_ABI_VOID __cilkrts_pop_frame(struct __cilkrts_stack_frame *sf);
extern CILK_ABI_VOID __cilkrts_enter_frame_1(__cilkrts_stack_frame *sf);
}

int fib(int n);

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

    if(n <= 2) {
        return n;
    }

    __cilkrts_stack_frame* sf = (__cilkrts_stack_frame*) alloca(sizeof(__cilkrts_stack_frame));;
    __cilkrts_enter_frame_1(sf);
    

    sf->flags |= CILK_FRAME_FUTURE_PARENT;

    cilk_fiber *volatile initial_fiber = cilk_fiber_get_current_fiber();
    
    cilk::future<int> x_fut = cilk::future<int>();
     
    __CILK_JUMP_BUFFER bkup;

    if (!CILK_SETJMP(sf->ctx)) {
        memcpy((void*)bkup, sf->ctx, 5*sizeof(void*));
        __cilkrts_switch_fibers(sf);

    } else if (sf->flags & CILK_FRAME_FUTURE_PARENT) {
        memcpy(sf->ctx, (void*)bkup, 5*sizeof(void*));
        fib_fut(&x_fut, n-1);

        cilk_fiber *fut_fiber = __cilkrts_pop_tail_future_fiber();

        __cilkrts_switch_fibers_back(sf, fut_fiber, initial_fiber);
    }


    if (sf->flags & CILK_FRAME_UNSYNCHED) {
        if (!CILK_SETJMP(sf->ctx)) {
            __cilkrts_future_sync(sf);
        }
    }

    y = fib(n - 2);
    x = x_fut.get();

    int _tmp = x+y;

    __cilkrts_pop_frame(sf);
    __cilkrts_leave_frame(sf);

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

    printf("Res: %d\n", res);

    if( TIMES_TO_RUN > 10 ) 
        print_runtime_summary(running_time, TIMES_TO_RUN); 
    else 
        print_runtime(running_time, TIMES_TO_RUN); 

    return 0;
}

