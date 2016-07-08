#include "uv.h"
#include <stdio.h>

void thread_one(void * arg){
    printf("I am thread no 1 \n");
    return;
}

void thread_two(void * arg){
    printf("I am thread no 2 \n");
    return;
}
int main(){
   int r, r1, r2;
   uv_thread_t th1, th2;
   
   uv_thread_create(&th1, thread_one, &r1);
   uv_thread_create(&th2, thread_two, &r2);
   uv_thread_join(&th1);
   uv_thread_join(&th2);
   return 0;
}