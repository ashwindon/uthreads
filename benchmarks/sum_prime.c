#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../mypthread.h"
#include <sys/time.h>

#define DEFAULT_THREAD_NUM 2
#define VECTOR_SIZE 3000000
//#define VECTOR_SIZE 30000
/* Global variables */
pthread_mutex_t   mutex;
int thread_num = 5;
pthread_t *thread;
int r[VECTOR_SIZE];
int s[VECTOR_SIZE];
int res = 0;
int maxPrime = 1000;

/* A CPU-bound task to do vector multiplication */
void prime_sum() {
	
	
	int i = 0;
	res = 0;
	pthread_mutex_lock(&mutex);
	for (i = 2; i <= maxPrime; i += 1) {
		int flag = 0;
		for(int j=2;j<i;j++) {
			if(i%j == 0) {
				flag = 1;
				break;
			}
		}
		if(flag == 0) {
			res+=i;
		}
	}
	pthread_mutex_unlock(&mutex);
	// for (i = n; i < VECTOR_SIZE; i += thread_num) {
	// 	//pthread_yield();
	// 	pthread_mutex_lock(&mutex);
	// 	res += r[i] * s[i];
	// 	//  printf("%d \n", res);
	// 	pthread_mutex_unlock(&mutex);	

	// }

	pthread_exit(NULL);
}

void verify() {
	int i = 0;
	res = 0;
	for (i = 2; i <= maxPrime; i += 1) {
		int flag = 0;
		for(int j=2;j<i;j++) {
			if(i%j == 0) {
				flag = 1;
				break;
			}
		}
		if(flag == 0) {
			res+=i;
		}
	}
	printf("verified res is: %d\n", res);
}

int main(int argc, char **argv) {
	
	int i = 0;




	// initialize pthread_t
	thread = (pthread_t*)malloc(thread_num*sizeof(pthread_t));


	pthread_mutex_init(&mutex, NULL);

	struct timespec start, end;
        clock_gettime(CLOCK_REALTIME, &start);

	for (i = 0; i < thread_num; ++i)
		pthread_create(&thread[i], NULL, &prime_sum, NULL);
	//printf("HERE AFTER CREATE\n");
	for (i = 0; i < thread_num; ++i)
		pthread_join(thread[i], NULL);
	// pritnf("HERE AFTER JOIN\n");

	clock_gettime(CLOCK_REALTIME, &end);
        printf("running time: %lu micro-seconds\n", 
	       (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000);
	printf("res is: %d\n", res);

	pthread_mutex_destroy(&mutex);
	verify();

	// Free memory on Heap
	free(thread);
	return 0;
}
