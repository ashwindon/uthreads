CC = gcc
CFLAGS = -g -w
all:: parallel_cal vector_multiply external_cal test dinningPhilosopher atomicityCheck sum_prime fibonacii

parallel_cal:
	$(CC) $(CFLAGS) -pthread -o parallel_cal parallel_cal.c -L../ -lmypthread

vector_multiply:
	$(CC) $(CFLAGS) -pthread -o vector_multiply vector_multiply.c -L../ -lmypthread

external_cal:
	$(CC) $(CFLAGS) -pthread -o external_cal external_cal.c -L../ -lmypthread

dinningPhilosopher:
	$(CC) $(CFLAGS) -pthread -o dinningPhilosopher dinningPhilosopher.c -L../ -lmypthread

atomicityCheck:
	$(CC) $(CFLAGS) -pthread -o atomicityCheck atomicityCheck.c -L../ -lmypthread

fibonacii:
	$(CC) $(CFLAGS) -pthread -o fibonacii fibonacii.c -L../ -lmypthread

sum_prime:
	$(CC) $(CFLAGS) -pthread -o sum_prime sum_prime.c -L../ -lmypthread	

test:
	$(CC) $(CFLAGS) -pthread -o test test.c -L../ -lmypthread
clean:
	rm -rf testcase test parallel_cal vector_multiply external_cal dinningPhilosopher fibonacii sum_prime atomicityCheck *.o ./record/