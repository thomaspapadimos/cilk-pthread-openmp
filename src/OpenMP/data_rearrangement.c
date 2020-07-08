#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "omp.h"
#include "utils.h"

#define DIM 3


void data_rearrangement(float *Y, float *X, 
			unsigned int *permutation_vector, 
			int N){
	  omp_set_num_threads(NUM_OF_THREADS);
    #pragma omp parallel for
	for(int i=0; i<N; i++){
    memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));
  }

}
