#define SIZE 100
#define NUM_ACCEL 4
#define OPS_PER_ACCEL SIZE/NUM_ACCEL
#define OMP_ACCEL 3
#include <stdio.h>
#include "prime.h"
#include <pthread.h>

int result[SIZE];

struct thread_data{
   int  startidx;
   int  maxidx;
};

void *prime(void *threadarg) {
	int num, i, j, k, tid;
	int isFactor=0, numFactors=0, isPrimeFactor=0, numPrimeFactors=0;
	int primeNumbers=0;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int n = arg->maxidx;
    //int startidx = (int)threadarg; 
	//loop through each input
	for (i=startidx; i<=n; i=i+NUM_ACCEL) {
		num = i;
        //if (num==0) continue;
	    numFactors = 0;
	    int temp[OMP_ACCEL] = {0, 0};
        //test each factor
	    #pragma omp parallel for num_threads(OMP_ACCEL) private(j, tid, isFactor) schedule(static, 1)
		for (j=2; j<=num; j++) {
            tid = omp_get_thread_num();
			isFactor = (num % j == 0);
            //printf("isFactor = %d\n", isFactor);
			temp[tid] = (isFactor==1)? temp[tid]+1 : temp[tid];
			//to calculate if the factor is a prime number
			//if (isFactor)
			//	printf("%d is a factor of %d\n", j, num);
			//	for (k=2; k<=j; k++) {
			//		isPrimeFactor = (j % k == 0);
			//		numPrimeFactors = (isPrimeFactor)? numPrimeFactors+1 : numPrimeFactors;
			//	}
			//}
		}
        for (j=0; j<OMP_ACCEL; j++) {
            numFactors += temp[j];
        } 
        //accumulate the number of prime numbers
		primeNumbers = (numFactors==1)? primeNumbers+1 : primeNumbers; 
		result[num] = numFactors;
		if (numFactors == 1)
		    printf("The number %d is a prime number\n", num);
		else
		   printf("The number %d is not a prime number\n", num);
	}
	//return primeNumbers;
	pthread_exit((void *)primeNumbers);     
}

int main() {
    legup_start_counter(0);
    int sum = 0;
    int limit = SIZE;
    int result[NUM_ACCEL];
	pthread_t threads[NUM_ACCEL];
	struct thread_data data[NUM_ACCEL];
    int i;
	for (i=0; i<NUM_ACCEL; i++) {
		//initialize structs to pass into accels
		data[i].startidx = i+1;
		data[i].maxidx = limit;
	}

    //fork the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, prime, (void *)&data[i]);
	}
	 
	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**) &result[i]);
	}
	
	for (i=0; i<NUM_ACCEL; i++) {
		sum += result[i];
	}
    int perf_counter = legup_stop_counter(0);
    printf("perf_counter = %d\n", perf_counter);

	printf("Sum = %d\n", sum);
	if (sum == 25)
		printf("PASS\n");
	else 
		printf("FAIL\n");
	
	
	return 0;
}
