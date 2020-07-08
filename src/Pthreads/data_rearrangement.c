#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "utils.h"
#include "pthread.h"

#define DIM 3

// creating a typedef struct 
typedef struct 
{
	float *Y,*X;
	unsigned int *permutation_vector;
    int N,start;
}R_DATA;

// Globally accesible variables 
R_DATA Rear_data;

void *rear_pthread (void *arg)
{
	long offset = (long)arg;  //extracting the data from the void pointer
	float *Y,*X;
	unsigned int *permutation_vector;
	Y=Rear_data.Y;
	X=Rear_data.X;
	permutation_vector=Rear_data.permutation_vector;
	int N = Rear_data.N;
	int start=offset*N/NUM_OF_THREADS;   // setting start point for each thread based on its id  where N/NUM_OF_THREADS is the step.
	int end=start+N/NUM_OF_THREADS;      // setting end point based on its id
	
	for(int i=start; i<end; i++){     //each thread will run this loop N/NUM_OF_THREADS times.
    memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));
	}
	pthread_exit((void*)0);  // we dont want to keep anything so we return NULL
}

void data_rearrangement(float *Y, float *X, 
			unsigned int *permutation_vector, 
			int N){
				
				
	int rc;
	//threads stuff, giving threads joinable attribute
	pthread_t threads[NUM_OF_THREADS];
	pthread_attr_t joinable;
    pthread_attr_init(&joinable);
    pthread_attr_setdetachstate(&joinable, PTHREAD_CREATE_JOINABLE);
    void *status;
	
	// passing the demanded data to global variable so we can access later for the thread work
	Rear_data.Y=Y;
	Rear_data.X=X;
	Rear_data.permutation_vector=permutation_vector;
	Rear_data.N=N;

	//creating the threads we requested
  for(long i=0; i<NUM_OF_THREADS; i++){
    
	rc=pthread_create(&threads[i], &joinable, rear_pthread, (void *)i);
	
	 if (rc) {                                                 // checking if the thread was created 
            printf("Error: pthread_create returned code %d\n", rc);
            return;
        }
  }
//free up memory and wait for threads to finish before returning
    pthread_attr_destroy(&joinable);
    for (int i = 0; i < NUM_OF_THREADS; i++)
    {
		
       int rc = pthread_join(threads[i], &status);
        if(rc) {                                              // checking if the thread ended its work                                           
            printf("Error: pthread_join returned code %d\n", rc);
            return;
        }
    }
}
