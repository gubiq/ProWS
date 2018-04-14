#include <iostream>
#include <stdio.h>
#include "cilk/cilk.h"
#include "../cilkplus-rts/include/internal/abi.h"
#include "../src/future.h"

#include <cstdint>

extern "C" {
extern void __assert_future_counter(int count);
extern void __print_curr_stack(char*);
extern void __assert_not_on_scheduling_fiber(void);
}

cilk::future<int>* test_future = NULL;
cilk::future<int>* test_future2 = NULL;
cilk::future<int>* test_future3 = NULL;
volatile int dummy = 0;
volatile int dummy2 = 0;
//porr::spinlock output_lock;

int helloFuture(void);
int helloMoreFutures(void);
void thread1(void);
void thread2(void);
void thread3(void);
void thread4(void);

int helloFuture() {
  for (volatile uint32_t i = 0; i < UINT32_MAX/8; i++) {
    dummy += i;
  }
  __assert_not_on_scheduling_fiber();
 // cilk_future_create(int,test_future3,helloMoreFutures);
  //cilk_future_get(test_future3);
  //delete test_future3;
 // __assert_future_counter(1);
  __print_curr_stack("\nhello future");
  return 42;
}

int helloMoreFutures() {
  for (volatile uint32_t j = 0; j < UINT32_MAX/4; j++) {
    dummy2 += j;
  }
  __assert_not_on_scheduling_fiber();
  __print_curr_stack("\nhello more futures");
  return 84;
}

void thread1() {
  __print_curr_stack("\nthread1 p1");
  __assert_not_on_scheduling_fiber();
  cilk_future_create(int,test_future,helloFuture);
  __assert_not_on_scheduling_fiber();
  __print_curr_stack("\nthread1 p2");
  auto result = cilk_future_get(test_future);
  __assert_not_on_scheduling_fiber();
  __print_curr_stack("\nthread1 p3");
}
void thread2() {
  printf("thread 2\n");
  //output_lock.lock();
  //output_lock.unlock();
  while (test_future==NULL);
  auto result = cilk_future_get(test_future);
  //output_lock.lock();
  printf("Thread  got %d\n", result);
  fflush(stdout);
  //output_lock.unlock();
}

void thread3() {
  __assert_not_on_scheduling_fiber();
  __print_curr_stack("\nthread3 p1");
  //cilk_future_create(int,test_future2,helloMoreFutures);
  reuse_future(int,test_future2,test_future,helloMoreFutures);
  __assert_not_on_scheduling_fiber();
  __print_curr_stack("\nthread3 p2");
  printf("%d\n", cilk_future_get(test_future2));
  __assert_not_on_scheduling_fiber();
  __print_curr_stack("\nthread3 p3");
}
void thread4() {
  printf("thread 4\n");
  //output_lock.lock();
  //output_lock.unlock();
  while (test_future2==NULL);
  auto result = cilk_future_get(test_future2);
  //output_lock.lock();
  printf("Thread  got %d\n", result);
  fflush(stdout);
  //output_lock.unlock();
}

//void run(void);

/*extern "C" {
int main(int argc, char * args[]) {
    run();
    return 0;
}
}*/

void is_only_printf_crashing() {
    int* dummy = (int*)alloca(sizeof(int)*10);
    assert(dummy);
    *dummy = 0xf00dface; 
    //printf("Hi!\n");
}

int main(int argc, char** argv) {
    //__print_curr_stack("initial");


    cilk_spawn thread1();
    printf("\n\nSimple Future Phase I Syncing\n\n");

    //for (int i = 0; i < 1; i++) {
    //    cilk_spawn thread2();
    //}
    //__print_curr_stack("pre-sync");

    cilk_sync;

    int* dummy1 = (int*)alloca(sizeof(int)*10);
    *dummy1 = 0xf00dface; 

    printf("\n\nSimple Future Phase II Commencing\n\n");
    //__print_curr_stack("\nphase II");

    cilk_spawn thread3();

    //for (int i = 0; i < 2; i++) {
    //    cilk_spawn thread4();
    //}

    //__print_curr_stack("\nphase II pre-sync");

    cilk_sync;

    //__print_curr_stack("end");

    int* dummy = (int*)alloca(sizeof(int)*10);
    *dummy = 0xf00dface; 
    delete test_future2;

    printf("That's all there is, and there isn't anymore...\n\n\n");
    return 0;
}
