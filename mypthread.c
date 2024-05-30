 // File: mypthread.c

    // List all group members' names:
    // iLab machine tested on:
    // #define _XOPEN_SOURCE 600
    #define QSIZE 10001
    #define tcbsize 101
    #define STACK_SIZE 16384
    #define QUANTUM 5
    #define Q_COUNT 4
    // #define POLICY "RR"



    #include <sys/time.h>
    #include <signal.h>
    #include "mypthread.h"
    #include "queue.h"


    // INITAILIZE ALL YOUR VARIABLES HERE
    // YOUR CODE HERE
    enum status {
      READY,
      RUNNING,
      BLOCKED,
      WAIT,
      TERMINATED,
      COMPLETED
    };
    struct Queue * runQueue;
    struct Queue * mlfq_Q[Q_COUNT];
    tcb * threadControlBlock[tcbsize];
    int tcbIndx = 0;
    ucontext_t Main, uct;
    mypthread_t * threads;
    int POLICY = 0;
    int count = 2;
    int scheduled = 0;
    int yieldThread = 0;
    int currThread = 1;
    int isQInitialized = 0;
    int qOpDone = 0;
    struct itimerval val, curr;
    int elapsed;
    int currQ = 0;
    int clockTicks = 0;

    void * threadInit(tcb * tcbcurr, void( * function)(void * ), void * arg);
    int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void * ( * function)(void * ), void * arg);
    int mypthread_join(mypthread_t thread, void ** value_ptr);
    void timeHelper(int signum);
    int roundRobin();  
    static void schedule();
    static void sched_RR();
    static void rr_MLFQ(int count);
    static void sched_MLFQ();
    static void sched_PSJF();

    void contextForScheduler() {
      tcb * schedulerTCB = (tcb * ) malloc(sizeof(tcb));
      schedulerTCB -> Id = 0;

      getcontext( & schedulerTCB -> context);
      schedulerTCB -> context.uc_stack.ss_sp = malloc(STACK_SIZE);
      schedulerTCB -> context.uc_stack.ss_size = STACK_SIZE;
      schedulerTCB -> context.uc_stack.ss_flags = 0;
      schedulerTCB -> context.uc_link = NULL;

      makecontext( & schedulerTCB -> context, (void( * )()) & schedule, 0);
      threadControlBlock[0] = schedulerTCB;
      //printf(" In contextForScheduler()\n");

    }

    void timerInit() {
      //printf("In Timer Init .....\n");
      if (signal(SIGALRM, (void( * )(int)) timeHelper) == SIG_ERR) {
        perror("Cant Raise SIGALRM");
        exit(1);
      }

      val.it_value.tv_sec = QUANTUM / 1000;
      val.it_value.tv_usec = (QUANTUM * 1000) % 1000000;
      val.it_interval = val.it_value;

      if (setitimer(ITIMER_REAL, & val, NULL) == -1) {
        perror("Error : setitimer()");
        exit(1);
      }
      //Timer doesnt start immediately so need to pause for a bit

      int flag = 0;
      while (flag == 0) {
        pause();
        flag = 1;
      }
    }

    void timeHelper(int signum) {

      // setupTimer expired, schedule next thread
      //printf("Timer called !! \n");
      if (currThread != 0 && qOpDone == 0) {
        //printf("0 -> %d \n", threadControlBlock[currThread] -> Id);

        swapcontext( & threadControlBlock[currThread] -> context, & threadControlBlock[0] -> context);
      }
    }
    void * threadInit(tcb * tcbcurr, void( * function)(void * ), void * arg) {
      function (arg);
      tcbcurr -> status = COMPLETED;

      free(tcbcurr -> context.uc_stack.ss_sp);
      // Switch context to scheduler
      setcontext( & threadControlBlock[0] -> context);

    }

    /* create a new thread */

    int mypthread_create(mypthread_t * thread, pthread_attr_t * attr, void * ( * function)(void * ), void * arg) {
      // YOUR CODE HERE
      //printf("In Create .....\n");
      if (isQInitialized == 0) {
        // int flag = 0;
          #ifdef PSJF
            POLICY = 1;
            // flag = 1;
          #endif

          #ifdef MLFQ
            POLICY = 2;
            // flag = 1;
          #endif      


        if(POLICY == 2)
        {
          for(int xi=0;xi<Q_COUNT;xi++) {
            mlfq_Q[xi] = createQueue(QSIZE);
          }
        }
        else if(POLICY == 0 || POLICY == 1) {
          runQueue = createQueue(QSIZE);
        }

        isQInitialized = 1;
      }
      // create a Thread Control Block
      // create and initialize the context of this thread
      // allocate heap space for this thread's stack
      // after everything is all set, push this thread into the ready queue

      //create tcb entry for current thread.
      * thread = count;
      //printf(" Thread ID - %d ", * thread);
      tcb * new_thread = (tcb * ) malloc(sizeof(tcb));
      new_thread -> status = READY;
      new_thread -> Id = count;
      new_thread -> priority = 0;
      new_thread -> quantumCount = 0;
      getcontext( & new_thread -> context);

      new_thread -> context.uc_stack.ss_sp = malloc(STACK_SIZE);
      new_thread -> context.uc_stack.ss_size = STACK_SIZE;
      new_thread -> context.uc_stack.ss_flags = 0;
      new_thread -> context.uc_link = NULL;

      // contextForScheduler();
      makecontext( & new_thread -> context, (void( * )()) & threadInit, 3, new_thread, function, arg);

      //printf(" The_thread->Id - %d ", new_thread -> Id);

      threadControlBlock[new_thread -> Id] = new_thread;
      count++;


      if (scheduled == 0) {
        //Initialize scheduler and Main Context.
        //printf(" Scheduler Initializer \n");
        contextForScheduler();
        tcb * mainTcb = (tcb * ) malloc(sizeof(tcb));
        mainTcb -> Id = 1;
        mainTcb -> status = READY;
        mainTcb -> priority = 0;
        mainTcb -> quantumCount = 0;
        threadControlBlock[1] = mainTcb;

        if(POLICY == 0 || POLICY == 1) {
          enqueue(runQueue, 1);
          enqueue(runQueue, new_thread -> Id);
        }
        else if(POLICY == 2) {
          enqueue(mlfq_Q[0], 1);
          enqueue(mlfq_Q[0], new_thread->Id);
        }
       
       
        scheduled = 1;

        // initialize periodic clock interrupt
        timerInit();

      }
      else
      {
        if(POLICY == 0 || POLICY == 1) {
          enqueue(runQueue, new_thread -> Id);
        }
        else if(POLICY == 2) {
          enqueue(mlfq_Q[0], new_thread->Id);
        }
      }

      return 0;
    };

    /* current thread voluntarily surrenders its remaining runtime for other threads to use */
    int mypthread_yield() {
      // YOUR CODE HERE

      // change current thread's state from Running to Ready
      // save context of this thread to its thread control block
      // switch from this thread's context to the scheduler's context
      
      //printf("yielding\n");
      tcb* curr = threadControlBlock[currThread];
      curr->status = READY;
      yieldThread = 1;
      swapcontext(&curr->context, &threadControlBlock[0]->context);
      return 0;
    };

    /* terminate a thread */
    void mypthread_exit(void * value_ptr) {
      // YOUR CODE HERE
      //printf("Exit called\n");
      // preserve the return value pointer if not NULL
      // deallocate any dynamic memory allocated when starting this thread
      tcb * currTcb = threadControlBlock[currThread];
            // printf("\nexited thread - %d  status - %d PRIORITY %d \n",currTcb->Id, currTcb -> status, currTcb->priority);

      // currTcb->retVal = value_ptr;
      free(currTcb -> context.uc_stack.ss_sp);

      if (currTcb -> waitCalledBy != 0) {
        tcb * joinedTCB = threadControlBlock[currTcb -> waitCalledBy];
        joinedTCB -> status = READY;
        if(POLICY == 2)
        {
          enqueue(mlfq_Q[threadControlBlock[joinedTCB -> Id]->priority], joinedTCB -> Id);
        }
        else if(POLICY == 0 || POLICY == 1) {
          enqueue(runQueue, joinedTCB -> Id);
        }
       
      }
            // printf("\nexited thread - %d  status - %d PRIORITY %d \n",currTcb->Id, currTcb -> status, currTcb->priority);

      currTcb -> status = COMPLETED;

      setcontext( & threadControlBlock[0] -> context);
      return;
    };

    /* Wait for thread termination */
    int mypthread_join(mypthread_t thread, void ** value_ptr) {
      // YOUR CODE HERE
      // wait for a specific thread to terminate
      // deallocate any dynamic memory created by the joining thread

      // printf("\nIn Join\n");


      tcb * currTcb = threadControlBlock[currThread];
      tcb * joinTcb = threadControlBlock[thread];
      // printf(" \nJOIN Thread Id %d ", joinTcb->Id);
      if (joinTcb -> status != COMPLETED) {
        joinTcb -> waitCalledBy = currTcb -> Id;
        currTcb -> status = BLOCKED;
      // printf(" \n Current on  %d \n", currTcb->Id);

      // printf(" \n Joined on  %d \n", joinTcb->Id);

        tcb * sched_tcb = threadControlBlock[0];
      swapcontext( & currTcb -> context, & sched_tcb -> context);
    }

      return 0;
    };
    /* initialize the mutex lock */
    int mypthread_mutex_init(mypthread_mutex_t * mutex, const pthread_mutexattr_t * mutexattr) {
      // YOUR CODE HERE

      //initialize data structures for this mutex

      mutex -> waitQueue = createQueue(QSIZE);
      mutex -> lock = 0;
      mutex->owner = 0;

      return 0;
    };

    /* aquire a mutex lock */
    int mypthread_mutex_lock(mypthread_mutex_t * mutex) {
      // YOUR CODE HERE
      qOpDone = 1;
      // use the built-in test-and-set atomic function to test the mutex
      // if the mutex is acquired successfully, return
      // if acquiring mutex fails, put the current thread on the blocked/waiting list and context switch to the scheduler thread
      while (__atomic_test_and_set((void * ) & mutex -> lock, __ATOMIC_RELAXED)) {
        //tcb* currTCB = get_current_tcb();
        tcb * sched_tcb = threadControlBlock[0];
        tcb * currTCB = threadControlBlock[currThread];
        enqueue(mutex -> waitQueue, currTCB -> Id);
        currTCB -> status = WAIT;
        //printf(" IS BLKOCKED - %d \n", currTCB->Id);
        qOpDone = 0;
        swapcontext( & currTCB -> context, & sched_tcb -> context);
        qOpDone = 1;
      }
      //Set current Thread as owner
      mutex -> owner = threadControlBlock[currThread] -> Id;
      qOpDone = 0;
      return 0;
    };

    /* release the mutex lock */
    int mypthread_mutex_unlock(mypthread_mutex_t * mutex) {
      qOpDone = 1;
      // YOUR CODE HERE

      // for (int xi = mutex->waitQueue -> front; xi <= mutex->waitQueue -> rear; xi++) {
      //   printf("%d ", mutex->waitQueue -> array[xi]);

      // }
      // update the mutex's metadata to indicate it is unlocked
      // put the thread at the front of this mutex's blocked/waiting queue in to the run queue
      tcb * currTcb = threadControlBlock[currThread];
      // if (currTcb -> Id != mutex -> lock) {
      //   printf("Can't Unlock!!\n");
      //   exit(1);
      // }

      if (mutex -> waitQueue -> size > 0) {

        // int waitedThread = mutex -> waitQueue -> array[mutex->waitQueue->front];
        //   threadControlBlock[waitedThread] -> status = READY;
        //   enqueue(runQueue, threadControlBlock[waitedThread] -> Id);
        //   mutex -> lock = 0;
       
        for (int i = mutex -> waitQueue -> front; i <= mutex -> waitQueue -> rear; i++) {
          // printf(" i = %d \n", )
          int waitedThread = mutex -> waitQueue -> array[i];
          threadControlBlock[waitedThread] -> status = READY;
          //Remove from Waiting Queue and add to Run Queue
          if(POLICY == 2)
          {
            enqueue(mlfq_Q[threadControlBlock[waitedThread]->priority], threadControlBlock[waitedThread] -> Id);
          }
          else if(POLICY == 0 || POLICY == 1) {
            enqueue(runQueue, threadControlBlock[waitedThread] -> Id);
          }


        }
        mutex -> lock = 0;
        removeAll(mutex->waitQueue);
      }
      else{
        mutex -> lock = 0;
        mutex->owner = 0;
        removeAll(mutex->waitQueue);
        // free(mutex->waitQueue);
        // mutex->waitQueue = createQueue(QSIZE);
      }
      qOpDone = 0;

      return 0;
    };

    /* destroy the mutex */
    int mypthread_mutex_destroy(mypthread_mutex_t * mutex) {
      // YOUR CODE HERE

      // deallocate dynamic memory allocated during mypthread_mutex_init
      // removeAll(mutex->waitQueue);
      // free(mutex->lock);
      // free(mutex->owner);
      free(mutex->waitQueue->array);
      return 0;
    };

    /* scheduler */
    static void schedule() {
      // YOUR CODE HERE

      // each time a timer signal occurs your library should switch in to this context

      // be sure to check the SCHED definition to determine which scheduling algorithm you should run
      //   i.e. RR, PSJF or MLFQ
      // printf("\nCurretnt Thread %d and priority %d \n", currThread, threadControlBlock[currThread]->priority);
      currQ = threadControlBlock[currThread]->priority;
      currThread = 0;
      // printf("Scheduler Initialized 1\n");
     
      if(POLICY == 2)
        sched_MLFQ();
      else if(POLICY == 0)
        sched_RR();
      else if(POLICY == 1){
        sched_PSJF();
      }
      return;
    }

    static void rr_MLFQ(int count) {
      // YOUR CODE HERE

      // Your own implementation of RR
      // (feel free to modify arguments and return types)
    //   printf("\nQUEUE ELEMENTS at Q %d - \n", 0);
     
    //   for(int r = mlfq_Q[0]->front;r<=mlfq_Q[0]->rear;r++) {
    //     printf("%d -> ",mlfq_Q[0]->array[r]);
    //   }
    //   printf("\n");


    //  printf("\nQUEUE ELEMENTS at Q %d - \n", 1);
     
    //   for(int r = mlfq_Q[1]->front;r<=mlfq_Q[1]->rear;r++) {
    //     printf("%d -> ",mlfq_Q[1]->array[r]);
    //   }
    //   printf("\n");


    //  printf("\nQUEUE ELEMENTS at Q %d - \n", 2);
     
    //   for(int r = mlfq_Q[2]->front;r<=mlfq_Q[2]->rear;r++) {
    //     printf("%d -> ",mlfq_Q[2]->array[r]);
    //   }
    //   printf("\n");

    //  printf("\nQUEUE ELEMENTS at Q %d - \n", 3);
     
    //   for(int r = mlfq_Q[3]->front;r<=mlfq_Q[3]->rear;r++) {
    //     printf("%d -> ",mlfq_Q[3]->array[r]);
    //   }
    //   printf("\n");

      if (mlfq_Q[count] -> size == 0) {

        printf("\nMLFQ Queue no %d Empty !\n", count);

        return ;
      }


      uint peeked = mlfq_Q[count] -> front;


      currThread = threadControlBlock[mlfq_Q[count] -> array[peeked]] -> Id;
      //Switch context to Current Thread
      setcontext( &threadControlBlock[currThread] -> context);

      return;
    }
    /* Round Robin scheduling algorithm */
    static void sched_RR() {
      // YOUR CODE HERE

      // Your own implementation of RR
      // (feel free to modify arguments and return types)
      //printf("In RR\n");

      if (runQueue -> size == 0) {

        printf("\nruQueue Empty !\n");

        return ;
      }
      mypthread_t curr = runQueue -> array[runQueue -> front];

      if (threadControlBlock[curr] -> status == COMPLETED || threadControlBlock[curr] -> status == BLOCKED ||threadControlBlock[curr] -> status == WAIT) {
        dequeue(runQueue);
        
      } else {
        mypthread_t popped = dequeue(runQueue);
        enqueue(runQueue, (int) popped);
      }


      uint peeked = runQueue -> front;
      currThread = threadControlBlock[runQueue -> array[peeked]] -> Id;
      setcontext( &threadControlBlock[currThread] -> context);

      return;
    }

    /* Preemptive PSJF (STCF) scheduling algorithm */
    static void sched_PSJF() {
      // YOUR CODE HERE
      // printf("In sched_PSJF\n");
      qOpDone = 1;
      // Your own implementation of PSJF (STCF)
      // (feel free to modify arguments and return types)
      if (runQueue -> size == 0) {

        printf("\nruQueue Empty !\n");

        return ;
      }

     
      mypthread_t curr = runQueue -> array[runQueue -> front];

      mypthread_t popped;
      if (threadControlBlock[curr] -> status == COMPLETED || threadControlBlock[curr] -> status == BLOCKED ||threadControlBlock[curr] -> status == WAIT) {
        threadControlBlock[curr]->quantumCount--; // decreasing quantum count so that next time this is scheduled again it gets more priority than the one already in queue if quantum count is same
        dequeue(runQueue);
      }

      // //Sort based on quantum count
      int n = runQueue->size;
      int first = runQueue->front;
      int last = runQueue->rear;
      for(int y=first; y<n; y++) {
        for(int z = first; z<n-y -1; z++) {
          int quantumCY = threadControlBlock[runQueue->array[z]]->quantumCount;
          int quantumCZ = threadControlBlock[runQueue->array[z+1]]->quantumCount;
          if(quantumCY > quantumCZ) {
            //swap
            int temp = runQueue->array[z];
            runQueue->array[z] = runQueue->array[z+1];
            runQueue->array[z+1] = temp;
          }
        }
      }

      threadControlBlock[popped]->quantumCount++;

      uint peeked = runQueue -> front;
      currThread = threadControlBlock[runQueue -> array[peeked]] -> Id;
      qOpDone = 0;
      setcontext( &threadControlBlock[currThread] -> context);



      return;
    }

    /* Preemptive MLFQ scheduling algorithm */
    /* Graduate Students Only */
    static void sched_MLFQ() {
      // YOUR CODE HERE
      //printf("In sched_MLFQ\n");
      // Your own implementation of MLFQ
      // (feel free to modify arguments and return types)
      // int currQ = threadControlBlock[mlfq_Q[currQ]->array[mlfq_Q[currQ]->front]]->priority;


      //Setting Quanta for different level.
      int quantaForCurrentLevel = (threadControlBlock[mlfq_Q[currQ]->array[mlfq_Q[currQ]->front]]->priority + 1) * QUANTUM;
      
      
      
      //PRIORITY INVERSION-
      if(clockTicks >= 10000) {
        int f = mlfq_Q[3]->front, l = mlfq_Q[3]->rear;
        for(int z = f;z<=l;z++) {
          enqueue(mlfq_Q[3], dequeue(mlfq_Q[3]));
        }
        clockTicks = 0;
      }
      
      int currStatus = threadControlBlock[mlfq_Q[currQ]->array[mlfq_Q[currQ]->front]]->status;

      if (currStatus == COMPLETED || currStatus == BLOCKED || currStatus == WAIT)
      {
        //Dequeue thread in case if status is marked BLOCKED, WAIT or COMPLETED.
        dequeue(mlfq_Q[currQ]);
       
      }
      else if(yieldThread == 1 || currQ >= Q_COUNT-1) {
        //decrease quantum Count to handle starvation
        uint t = mlfq_Q[currQ]->array[mlfq_Q[currQ]->front];
        threadControlBlock[t]->quantumCount++;
        enqueue(mlfq_Q[currQ], dequeue(mlfq_Q[currQ]));

      } else {
        //decrease quantum Count to handle starvation
        uint t = mlfq_Q[currQ]->array[mlfq_Q[currQ]->front];
        threadControlBlock[t]->quantumCount++;
        threadControlBlock[mlfq_Q[currQ]->array[mlfq_Q[currQ]->front]]->priority = currQ + 1;
        enqueue(mlfq_Q[currQ+1], dequeue(mlfq_Q[currQ]));
      }
      yieldThread = 0;

      int x = 0;
      while (x < Q_COUNT) {
          if(mlfq_Q[x]->size != 0) {
            //Call Round Robin for each queue in MLFQ.
            rr_MLFQ(x);
          }
         clockTicks++;
          x++;
      }

      return;
    }