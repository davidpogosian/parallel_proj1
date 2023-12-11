#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#include "oddevenm.h"
#include "incOrder.h"

void oddevenm(int* subarr, int subarr_size, int world_rank, int world_size) {

    /* figuring out partners for subarray exchanges */
    int odd_partner, even_partner;
    if (world_rank % 2 == 0) {
        even_partner = world_rank + 1;
        odd_partner = world_rank - 1;
    } else {
        even_partner = world_rank - 1;
        odd_partner = world_rank + 1;
    }
    if (even_partner == -1 || even_partner == world_size) {
        even_partner = MPI_PROC_NULL;
    }
    if (odd_partner == -1 || odd_partner == world_size) {
        odd_partner = MPI_PROC_NULL;
    }

    /* prepare for echange */
    int double_subarr_size = subarr_size * 2;
    int* double_subarr = (int*) malloc(sizeof(int) * double_subarr_size);
    int incoming_subarr_size = subarr_size;
    int* incoming_subarr = (int*) malloc(sizeof(int) * subarr_size);

    /* initialize old_subarr and halt flag */
    int old_subarr_size = subarr_size;
    int* old_subarr = (int*) malloc(sizeof(int) * old_subarr_size);
    for (int i = 0; i < old_subarr_size; ++i) {
        old_subarr[i] = subarr[i];
    }
    int halt;

    /* mainloop */
    MPI_Status status;
    for (int i = 0; i < world_size - 1; ++i) {
        halt = 1;
        int current_partner;
        if (i % 2 == 0) {
            /* even phase */
            current_partner = even_partner;
        } else {
            /* odd phase */
            current_partner = odd_partner;
        }

        if (current_partner != MPI_PROC_NULL) {
            MPI_Sendrecv(subarr, subarr_size, MPI_INT, current_partner, 0, incoming_subarr, incoming_subarr_size, MPI_INT, current_partner, 0, MPI_COMM_WORLD, &status);

            /* populate double subarray = [subarr, incoming_subarr] */
            for (int j = 0; j < subarr_size; ++j) {
                double_subarr[j] = subarr[j];
                double_subarr[j + subarr_size] = incoming_subarr[j];
            }

            /* sort double subarray */
            qsort(double_subarr, double_subarr_size, sizeof(int), incOrder);

            /* select half of double subarray to keep */
            if (world_rank > current_partner) {
                // keep greater half
                for (int j = 0; j < subarr_size; ++j) {
                    subarr[j] = double_subarr[j + subarr_size];
                }
            } else {
                // keep lesser half
                for (int j = 0; j < subarr_size; ++j) {
                    subarr[j] = double_subarr[j];
                }
            }
        }

        /* update old subarray and check if subarray changed */
        for (int j = 0; j < subarr_size; ++j) {
            if (subarr[j] != old_subarr[j]) {
                halt = 0;
            }
            old_subarr[j] = subarr[j];
        }

        MPI_Allreduce(&halt, &halt, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
        if (halt) {
            break;
        }
    }

    free(incoming_subarr);
    free(double_subarr);
    free(old_subarr);
}