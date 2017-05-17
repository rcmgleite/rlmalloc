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


  /*******************************************************/
  /*******************************************************/
  /****************** RANDOM ALLOCATIONS *****************/
  /*******************************************************/
  /*******************************************************/
  printf("Random allocs\n");
  char* var1 = (char*) rlmalloc(sizeof(*var1) * 10);
  char* var2 = (char*) rlmalloc(sizeof(*var2) * 11);
  char* var3 = (char*) rlmalloc(sizeof(*var3) * 12);
  char* var4 = (char*) rlmalloc(sizeof(*var4) * 13);
  char* var5 = (char*) rlmalloc(sizeof(*var5) * 14);
  
  printf("Some free...\n");
  rlfree(var1);
  rlfree(var2);
  rlfree(var3);

  printf("More random allocs\n");
  char* var6 = (char*) rlmalloc(sizeof(*var6) * 3);
  char* var7 = (char*) rlmalloc(sizeof(*var7) * 4);
  char* var8 = (char*) rlmalloc(sizeof(*var8) * 5);

  printf("Final frees\n");
  rlfree(var4);
  rlfree(var5);
  rlfree(var6);
  rlfree(var7);
  rlfree(var8);
  
  printf("DONE!\n");

  return 0;
}
