#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "utils.h"
#include "pthread.h"

#define MAXBINS 8

inline void swap_long(unsigned long int **x, unsigned long int **y){

  unsigned long int *tmp;
  tmp = x[0];
  x[0] = y[0];
  y[0] = tmp;

}

inline void swap(unsigned int **x, unsigned int **y){

  unsigned int *tmp;
  tmp = x[0];
  x[0] = y[0];
  y[0] = tmp;

}

// creating a typedef struct 
typedef struct{
    unsigned long int *morton_codes;
    unsigned long int *sorted_morton_codes;
    unsigned int *permutation_vector;
    unsigned int *index;
    unsigned int *level_record;
    int N;
    int population_threshold;
    int sft;
    int lv;
}Radix_Sort_Data;

void *Radix_Sort_pthread(void* arg){
  Radix_Sort_Data *pthread_data = (Radix_Sort_Data*) arg;  // extracting the data from the void pointer 
  // calling the truncated_radix_sort recursively with the new data for each thread 
  truncated_radix_sort(pthread_data->morton_codes,   
                       pthread_data->sorted_morton_codes,
                       pthread_data->permutation_vector,
                       pthread_data->index,
                       pthread_data->level_record,
                       pthread_data->N,
                       pthread_data->population_threshold,
                       pthread_data->sft,
                       pthread_data->lv);
  pthread_exit(0);
}

void truncated_radix_sort(unsigned long int *morton_codes,
			  unsigned long int *sorted_morton_codes,
			  unsigned int *permutation_vector,
			  unsigned int *index,
			  unsigned int *level_record,
			  int N,
			  int population_threshold,
			  int sft, int lv)
{
  int BinSizes[MAXBINS]={0};
  int BinCursor[MAXBINS]={0};
  unsigned int *tmp_ptr;
  unsigned long int *tmp_code;

  if(N<=0)
  {
    return;
  }
  else if(N<=population_threshold || sft < 0) { // Base case. The node is a leaf
    level_record[0] = lv; // record the level of the node
    memcpy(permutation_vector, index, N*sizeof(unsigned int)); // Copy the pernutation vector
    memcpy(sorted_morton_codes, morton_codes, N*sizeof(unsigned long int)); // Copy the Morton codes
    return;
  }
  else
  {
     level_record[0] = lv;

     
       // Find which child each point belongs to
         for(int j=0; j<N; j++){
             unsigned int ii = (morton_codes[j]>>sft) & 0x07;
             BinSizes[ii]++;
         }

           // scan prefix changed code
	 
    for(int i=1; i<MAXBINS; i++){
      BinCursor[i] = BinCursor[i-1]+BinSizes[i-1];
	}	 
	 for(int i=1; i<MAXBINS; i++)
	{
      BinSizes[i] = BinSizes[i]+BinCursor[i];
    }
         for(int j=0; j<N; j++){
            unsigned int ii = (morton_codes[j]>>sft) & 0x07;
            permutation_vector[BinCursor[ii]] = index[j];
            sorted_morton_codes[BinCursor[ii]] = morton_codes[j];
            BinCursor[ii]++;
         }
         //swap the index pointers
         swap(&index, &permutation_vector);
         //swap the code pointers
         swap_long(&morton_codes, &sorted_morton_codes);

		 //threads stuff, giving threads joinable attribute
         pthread_t rthreads[MAXBINS];
         pthread_attr_t attribute;
         pthread_attr_init(&attribute);
         pthread_attr_setdetachstate(&attribute,PTHREAD_CREATE_JOINABLE);
         Radix_Sort_Data Radix_data[MAXBINS];

         void *status;
         /* Call the function recursively to split the lower levels */
         for(int i=0; i<MAXBINS; i++){
               int offset = (i>0) ? BinSizes[i-1] : 0;
               int size = BinSizes[i] - offset;
               if(lv<1) 
               {
				   // passing the demanded data to stuct array, so each thread to have the right size of data neaded to do its work 
                 Radix_data[i].morton_codes =&morton_codes[offset];
                 Radix_data[i].sorted_morton_codes = &sorted_morton_codes[offset];
                 Radix_data[i].permutation_vector = &permutation_vector[offset];
                 Radix_data[i].index = &index[offset];
                 Radix_data[i].level_record = &level_record[offset];
                 Radix_data[i].N = size;
                 Radix_data[i].population_threshold = population_threshold;
                 Radix_data[i].sft =  sft-3;
                 Radix_data[i].lv = lv+1;
               }
         }
      if(lv<1) // we set the tree depth to be 1 so threads do their work faster
      {
		  // creating 8 threads as maximum number of branches 
          for(int i=0; i<MAXBINS; i++){
               int rc = pthread_create(&rthreads[i], &attribute, Radix_Sort_pthread,(void*) &Radix_data[i]);
                 if(rc){                                                 // checking if the thread was created
                       printf("ERROR returned code from pthreat_create() is %d\n",rc);
                       return;
                 }
          }

	 //free up memory and wait for threads to finish before returning
      pthread_attr_destroy(&attribute);

      for(int i=0; i<MAXBINS; i++) {
         int rc = pthread_join(rthreads[i], &status);
          if (rc) {                                                       // checking if the thread ended its work 
                 printf("ERROR; return code from pthread_join() is %d\n", rc); 
                return;
                }
       }
     }
    
}
}
