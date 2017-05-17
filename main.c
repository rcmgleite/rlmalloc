#include <stdio.h>
#include <string.h>

#include "rlmalloc.h"

#define MY_NAME "rafael"
#define SMALL_STR "tiny"

int main() {
  
  /*******************************************************/
  /*******************************************************/
  /***************** FIRST MEMORY ALLOC ******************/
  /*******************************************************/
  /*******************************************************/
  size_t ARR_SIZE = 2;
  
  printf("First memory allocation will take place NOW!\n");
  int* arr = (int*) rlmalloc(sizeof(*arr) * ARR_SIZE);
  if (!arr) {
    printf("rlmalloc sucks!\n");
    return 1;
  }

  printf("First memory allocation DONE!\n");

  for (size_t i = 0; i < ARR_SIZE; i++) {
    arr[i] = i * 10;
  }

  printf("First memory allocation: Printing array: \n");
  for (size_t i = 0; i < ARR_SIZE; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");
  
  /*******************************************************/
  /*******************************************************/
  /***************** SECOND MEMORY ALLOC *****************/
  /*******************************************************/
  /*******************************************************/
  size_t MY_NAME_SIZE = sizeof(MY_NAME);

  printf("Second memory allocation will take place NOW!\n");
  char* my_name = (char*) rlmalloc(sizeof(*my_name) * MY_NAME_SIZE);
  if (!my_name) {
    printf("rlmalloc sucks!\n");
    return 1;
  }

  const char* aux = MY_NAME;
  memcpy(my_name, aux, MY_NAME_SIZE);

  printf("Second memory allocation: Printing my_name\n");
  printf("%s", my_name);

  /*******************************************************/
  /*******************************************************/
  /********** DEALLOC FIRST AND ALLOC THIRD  *************/
  /*******************************************************/
  /*******************************************************/
  
  printf("\nFirst memory allocation: Freeing array\n");
  rlfree(arr);
  printf("First memory allocation: Free DONE!\n");
  
  size_t SMALL_STR_SIZE = sizeof(SMALL_STR);
  printf("Third memory allocation will take place NOW!\n");
  char* small_str = (char*) rlmalloc(sizeof(*small_str) * SMALL_STR_SIZE);
   if (!small_str) {
    printf("rlmalloc sucks!\n");
    return 1;
  }
  
  const char* small_str_aux = SMALL_STR;
  memcpy(small_str, small_str_aux, SMALL_STR_SIZE);

  printf("Third memory allocation: Printing small_str\n");
  printf("%s", small_str);

  printf("\nThird memory allocation: Freeing small_str\n");
  rlfree(small_str);
  printf("Third memory allocation: Free DONE!\n");

  printf("AAAAAAAAAAAA%s\n", my_name);

  printf("\nSecond memory allocation: Freeing my_name\n");
  rlfree(my_name);
  printf("Second memory allocation: Free DONE!\n");


  return 0;
}
