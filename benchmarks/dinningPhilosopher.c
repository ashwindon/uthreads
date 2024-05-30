#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../mypthread.h"
#include <sys/time.h>

pthread_t philosopher[5];
pthread_mutex_t chopstick[5];
int noOfPhilosophers=0;
void *func(int n)
   {

//    printf ("Philosopher %d is thinking\n",n);

   pthread_mutex_lock(&chopstick[n]);
   pthread_mutex_lock(&chopstick[(n+1)%5]);
//    printf ("Philosopher %d is eating\n",n);
   sleep(2);
   pthread_mutex_unlock(&chopstick[n]);
   pthread_mutex_unlock(&chopstick[(n+1)%5]);
        noOfPhilosophers++;

   printf ("Philosopher %d finished eating\n",n);

   pthread_exit(NULL);
   }

int main()
   {
   int i;
   	struct timespec start, end;

   for(i=0;i<5;i++)
      pthread_mutex_init(&chopstick[i],NULL);

    clock_gettime(CLOCK_REALTIME, &start);

   for(i=0;i<5;i++)
      pthread_create(&philosopher[i],NULL,(void *)func,(void *)i);

   for(i=0;i<5;i++)
      pthread_join(philosopher[i],NULL);

	clock_gettime(CLOCK_REALTIME, &end);
    printf("running time: %lu micro-seconds\n", (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000);

    if(noOfPhilosophers == 5) 
    {
        printf("All philosophers are done eating\n");
    }
            
   for(i=0;i<5;i++)
      pthread_mutex_destroy(&chopstick[i]);

   return 0;
   }