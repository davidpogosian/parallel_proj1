#include <stdlib.h>
#include <time.h>

#include "generateArray.h"

int* generateArray(int size, int low, int high) {
    int* arr = (int*) malloc(sizeof(int) * size);
    for (int i = 0; i < size; ++i) {
        srand(time(NULL) + i);
        arr[i] = low + rand() % (high - low + 1);
    }
    return arr;
}


// 1 - 5

// 1 + rand() % (5 - 1 + 1) 