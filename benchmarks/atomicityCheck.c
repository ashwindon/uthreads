#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../mypthread.h"
#include <sys/time.h>

#define DEFAULT_THREAD_NUM 2
#define VECTOR_SIZE 3000000

/* Global variables */
pthread_mutex_t   mutex;
int thread_num;
int* counter;
pthread_t *thread;
int r[VECTOR_SIZE];
int finalBalance = 10000000;

/* A CPU-bound task to do vector multiplication */
void calFinalBalance(void* arg) {
	int i = 0;
	int n = *((int*) arg);
	pthread_yield();
	for (i = n; i < VECTOR_SIZE; i += thread_num) {
		pthread_mutex_lock(&mutex);
        if(i %2 == 0) {
            finalBalance+=r[i];
        }	
        else{
            finalBalance-=r[i];
        }
		pthread_mutex_unlock(&mutex);	

	}

	pthread_exit(NULL);
}

void verify() {
	int i = 0;
	finalBalance = 10000000;
	for (i = 0; i < VECTOR_SIZE; i += 1) {
		if(i %2 == 0) {
            finalBalance+=r[i];
        }	
        else{
            finalBalance-=r[i];
        }
	}
	printf("verified finalBalance is: %d\n", finalBalance);
}

int main(int argc, char **argv) {
	
	int i = 0;

	if (argc == 1) {
		thread_num = DEFAULT_THREAD_NUM;
	} else {
		if (argv[1] < 1) {
			printf("enter a valid thread number\n");
			return 0;
		} else {
			thread_num = atoi(argv[1]);
		}
	}

	// initialize counter
	counter = (int*)malloc(thread_num*sizeof(int));
	for (i = 0; i < thread_num; ++i)
		counter[i] = i;

	// initialize pthread_t
	thread = (pthread_t*)malloc(thread_num*sizeof(pthread_t));

	// initialize data array
	for (i = 0; i < VECTOR_SIZE; ++i) {
		r[i] = i+1;
            
	}

	pthread_mutex_init(&mutex, NULL);

	struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

	for (i = 0; i < thread_num; ++i)
		pthread_create(&thread[i], NULL, &calFinalBalance, &counter[i]);
	for (i = 0; i < thread_num; ++i)
		pthread_join(thread[i], NULL);

	clock_gettime(CLOCK_REALTIME, &end);
    printf("running time: %lu micro-seconds\n", (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000);
	printf("finalBalance is: %d\n", finalBalance);

	pthread_mutex_destroy(&mutex);
	verify();

	// Free memory on Heap
	free(thread);
	free(counter);
	return 0;
}
