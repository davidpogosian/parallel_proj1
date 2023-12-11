

#include "incOrder.h"

int incOrder(const void* e1, const void* e2) {
    return (*((int *)e1) - *((int *) e2));
}