#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "generateArray.h"
#include "oddeven.h"
#include "oddevenm.h"
#include "incOrder.h"

FILE* graph_data_fp;

int main(int argc, char** argv) {
	int* random_arr;
	int* oddeven_arr;
	int* oddevenm_arr;
	int* qsort_arr;
	int* subarr;
	int world_size, world_rank, arr_size, subarr_size;
	double oddeven_start, oddeven_end, oddevenm_start, oddevenm_end, qsort_start, qsort_end;
	double oddeven_sum, oddevenm_sum, qsort_sum;

    MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	/* initializing file pointers */
	if (world_rank == 0) {
		graph_data_fp = fopen("graph_data.txt", "w");
	}

	for (int count = 65536; count < 16777216 + 1; count *= 16) {
		oddeven_sum = 0;
		oddevenm_sum = 0;
		qsort_sum = 0;
		for (int rep = 0; rep < 5; ++rep) {
			/* generate random array  */
			if (world_rank == 0) {
				arr_size = count;
				random_arr = generateArray(arr_size, 0, 128);
				oddeven_arr = (int*) malloc(sizeof(int) * arr_size);
				oddevenm_arr = (int*) malloc(sizeof(int) * arr_size);
				qsort_arr = (int*) malloc(sizeof(int) * arr_size);
				for (int i = 0; i < arr_size; ++i) {
					oddeven_arr[i] = random_arr[i];
					oddevenm_arr[i] = random_arr[i];
					qsort_arr[i] = random_arr[i];
				}
			}

			/* prepare for subarray distribution */
			MPI_Bcast(&arr_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
			subarr_size = arr_size / world_size;
			subarr = (int*) malloc(sizeof(int) * subarr_size);

			/* oddeven run */
			oddeven_start = MPI_Wtime();
			MPI_Scatter(oddeven_arr, subarr_size, MPI_INT, subarr, subarr_size, MPI_INT, 0, MPI_COMM_WORLD);
			oddeven(subarr, subarr_size, world_rank, world_size);
			MPI_Gather(subarr, subarr_size, MPI_INT, oddeven_arr, subarr_size, MPI_INT, 0, MPI_COMM_WORLD);
			oddeven_end = MPI_Wtime();
			
			/* oddevenm run */
			oddevenm_start = MPI_Wtime();
			MPI_Scatter(oddevenm_arr, subarr_size, MPI_INT, subarr, subarr_size, MPI_INT, 0, MPI_COMM_WORLD);
			oddevenm(subarr, subarr_size, world_rank, world_size);
			MPI_Gather(subarr, subarr_size, MPI_INT, oddevenm_arr, subarr_size, MPI_INT, 0, MPI_COMM_WORLD);
			oddevenm_end = MPI_Wtime();

			/* qsort run */
			if (world_rank == 0) {
				qsort_start = MPI_Wtime();
				qsort(qsort_arr, arr_size, sizeof(int), incOrder);
				qsort_end = MPI_Wtime();
			}

			oddeven_sum += oddeven_end - oddeven_start;
			oddevenm_sum += oddevenm_end - oddevenm_start;
			qsort_sum += qsort_end - qsort_start;

			/* deallocate memory */
			if (world_rank == 0) {
				free(random_arr);
				free(oddeven_arr);
				free(oddevenm_arr);
				free(qsort_arr);
			}
			free(subarr);
		}

		if (world_rank == 0) {
			fprintf(graph_data_fp, "%d %f %f %f \n", count, oddeven_sum / 5, oddevenm_sum / 5, qsort_sum / 5);
		}
	}

	MPI_Finalize();
    return 0;
}