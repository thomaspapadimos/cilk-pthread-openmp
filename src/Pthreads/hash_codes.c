#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "float.h"
#include "utils.h"
#include "pthread.h"

#define DIM 3

// creating a typedef struct 
typedef struct 
{
	unsigned int *codes;
	float *X,*low,step;
	int N;
}Q_data;

// Globally accesible variables 
Q_data Quant_data;

void *Hash_pthread (void *arg)
{
	long offset = (long)arg;  //extracting the data from the void pointer
	unsigned int *codes;
	float *X,*low,step;
	int N,start,end;
	codes = Quant_data.codes;
	X = Quant_data.X;
	low = Quant_data.low;
	step = Quant_data.step;
	N = Quant_data.N;
	start = offset*N/NUM_OF_THREADS;  // setting start point for each thread based on its id  where N/NUM_OF_THREADS is the step.
	end = start+N/NUM_OF_THREADS;     // setting end point based on its id 
	 for (int i =start; i < end; i++){  //each thread will run this loop N/NUM_OF_THREADS times.
	for(int j=0; j<DIM; j++){
      codes[i*DIM + j] = compute_code(X[i*DIM + j], low[j], step); 
        }
    }
	pthread_exit((void*)0); // we dont want to keep anything so we return NULL
}


inline unsigned int compute_code(float x, float low, float step){

  return floor((x - low) / step);

}


/* Function that does the quantization */
void quantize(unsigned int *codes, float *X, float *low, float step, int N){

     int rc;
 //threads stuff, giving threads joinable attribute
    pthread_t threads[NUM_OF_THREADS];
    pthread_attr_t joinable;
    pthread_attr_init(&joinable);
    pthread_attr_setdetachstate(&joinable, PTHREAD_CREATE_JOINABLE);
    void *status;

	// passing the demanded data to global variable so we can access later for the thread work 
    Quant_data.codes = codes;
    Quant_data.X = X;
    Quant_data.low = low;
    Quant_data.step = step;
    Quant_data.N = N;
	
	//creating the threads we requested
  for(long i=0; i<NUM_OF_THREADS; i++){
	  
	  
      rc=pthread_create(&threads[i], &joinable, Hash_pthread, (void *)i); 
		 if (rc) {                                              // checking if the thread was created 
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
float max_range(float *x){

  float max = -FLT_MAX;
  for(int i=0; i<DIM; i++){
    if(max<x[i]){
      max = x[i];
    }
  }

  return max;

}

void compute_hash_codes(unsigned int *codes, float *X, int N, 
			int nbins, float *min, 
			float *max){
  
  float range[DIM];
  float qstep;

  for(int i=0; i<DIM; i++){
    range[i] = fabs(max[i] - min[i]); // The range of the data
    range[i] += 0.01*range[i]; // Add somthing small to avoid having points exactly at the boundaries 
  }

  qstep = max_range(range) / nbins; // The quantization step 
  
  quantize(codes, X, min, qstep, N); // Function that does the quantization

}



