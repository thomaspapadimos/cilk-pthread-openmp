#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "utils.h"
#include "pthread.h"

#define DIM 3

// creating a typedef struct 
typedef struct 
{
	unsigned long int *mcodes;
    unsigned int *codes;
    int N;
} M_DATA ;

// Globally accesible variable 
M_DATA Morton_data;

inline unsigned long int splitBy3(unsigned int a){
    unsigned long int x = a & 0x1fffff; // we only look at the first 21 bits
    x = (x | x << 32) & 0x1f00000000ffff;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
    x = (x | x << 16) & 0x1f0000ff0000ff;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
    x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    x = (x | x << 2) & 0x1249249249249249;
    return x;
}

inline unsigned long int mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z){
    unsigned long int answer;
    answer = splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
    return answer;
}

void *Morton_pthread (void *arg)
{
	//passing data from the global variable 
	long offset =(long)arg; //extracting the data from the void pointer
	unsigned long int *mcodes; 
    unsigned int *codes;
	mcodes=Morton_data.mcodes; 
	codes=Morton_data.codes;
    int N=Morton_data.N;
	int start = offset*N/NUM_OF_THREADS;  // setting start point for each thread based on its id. 
	int end = start+N/NUM_OF_THREADS;    // setting end point based on its id .
	 for (int i =start; i < end; i++)    //each thread will run this loop N/NUM_OF_THREADS times.
    {
	// Compute the morton codes using the magic bits method
        mcodes[i] = mortonEncode_magicbits(codes[i*DIM], codes[i*DIM + 1], codes[i*DIM + 2]);
	}
	pthread_exit((void*)0); // we dont want to keep anything so we return NULL
}

/* The function that transform the morton codes into hash codes */ 
void morton_encoding(unsigned long int *mcodes, unsigned int *codes, int N, int max_level){
	int rc;
  //threads stuff, giving threads joinable attribute
    pthread_t threads[NUM_OF_THREADS];
    pthread_attr_t joinable;       
    pthread_attr_init(&joinable);
    pthread_attr_setdetachstate(&joinable, PTHREAD_CREATE_JOINABLE);
    void *status;
	
	// passing the demanded data to global variable so we can access later for the thread work 
     Morton_data.mcodes = mcodes; 
     Morton_data.codes = codes; 
     Morton_data.N =N;  
	
	//creating the threads we requested 
  for(long i=0; i<NUM_OF_THREADS; i++){
	    
	     
		 rc=pthread_create(&threads[i], &joinable, Morton_pthread, (void *)i); 
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


