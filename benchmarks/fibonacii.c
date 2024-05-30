#include "pthread.h"
#include <stdio.h>
#include <stdlib.h>
#include "../mypthread.h"

int sum; /* this data is shared by the thread(s) */
void *runner(void *param); /* the thread */
int counter = 30;
pthread_mutex_t   mutex;

int main()
{
	int i;
  
	pthread_attr_t attr; 
    struct timespec start, end;
	// pthread_attr_init(&attr);
    pthread_mutex_init(&mutex, NULL);
        clock_gettime(CLOCK_REALTIME, &start);

	for(i=0;i<=counter;i++){
		pthread_t thread;
		pthread_create(&thread,NULL,runner,(void*)i);
		pthread_join(thread,NULL);	
	}
    	clock_gettime(CLOCK_REALTIME, &end);
        printf("running time: %lu micro-seconds\n", 
	       (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000);
    printf("Final element of Fibonacci sequence %d \n", sum);

    verify();

    pthread_mutex_destroy(&mutex);

}

void verify() {
	// for(int i=0;i<=counter;i++){
	// 	sum = fibonacci((int)i);
	// }
    sum = fibonacci((int)counter);
	printf("verified sum is: %d\n", sum);
}
void *runner(void *param)
{
    pthread_mutex_lock(&mutex);
	sum = fibonacci((int)param);
    pthread_mutex_unlock(&mutex);
	pthread_exit(0);
}

int fibonacci (int x)
{
    if (x <= 1) {
        return 1;
    }
    return fibonacci(x-1) + fibonacci(x-2);
}