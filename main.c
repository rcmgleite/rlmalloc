#include <stdio.h>
#include <string.h>

#include "rlmalloc.h"

#define MY_NAME "rafael"

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
  /********************** FREE ZONE **********************/
  /*******************************************************/
  /*******************************************************/
  printf("\nSecond memory allocation: Freeing my_name\n");
  rlfree(my_name);
  printf("Second memory allocation: Free DONE!");

  printf("\nFirst memory allocation: Freeing array\n");
  rlfree(arr);
  printf("First memory allocation: Free DONE!");

  return 0;
}
