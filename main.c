#include <stdio.h>

#include "rlmalloc.h"

int main() {
  size_t arr_size = 50;
  int* arr = (int*) rlmalloc(sizeof(*arr) * arr_size);

  for (size_t i = 0; i < arr_size; i++) {
    arr[i] = i * 10;
  }

  for (size_t i = 0; i < arr_size; i++) {
    printf("%d ", arr[i]);
  }

  rlfree(arr);
  return 0;
}
