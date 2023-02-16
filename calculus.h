// Profeanu Ioana, 333CA
// calculus header file
#ifndef CALCULUS
#define CALCULUS

#include "tema3.h"

// function which makes the calculus on the array
void make_calculus(int rank, int leader, int no_processes, int array_length,
    int communication_error, MPI_Status status, int total_workers,
    int coordinator_parent, int coordinator_child, int *calculus_array,
    vector<int> subordinate_workers, vector<int> clusters_coordinators_order,
    int** topology)
{
    // if the process is coordinator and it is NOT the proccess with rank 1
    // when we have the second case of communication error
    if (is_coordinator(rank) == true && (rank == 1
        && communication_error == 2) == 0) {
        // receive data about the array from parent
        if (coordinator_parent != -1 && rank != 0) {
            // receive array length
            MPI_Recv(&array_length, 1, MPI_INT, coordinator_parent,
                0, MPI_COMM_WORLD, &status);
            // receive array
            calculus_array = (int*)malloc(sizeof(int) * array_length);
            MPI_Recv(calculus_array, array_length, MPI_INT, coordinator_parent,
                    1, MPI_COMM_WORLD, &status);
        }

        // send array data to child
        if (coordinator_child != -1) {
            // send array length
            MPI_Send(&array_length, 1, MPI_INT, coordinator_child,
                    0, MPI_COMM_WORLD);
            // send array
            MPI_Send(calculus_array, array_length, MPI_INT, coordinator_child,
                    1, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << coordinator_child << ")\n";
        }

        // get the number of working clusters that the coordinators
        // prior to the current coordinators have in total
        int prior_workers_other_clusters = 0;
        // for each coordinator (in the established chain order)
        for (int i = 0; i < (int)clusters_coordinators_order.size(); i++) {
            // check if we reached the current process
            if (clusters_coordinators_order[i] == rank) {
                break;
            }
            // if the coordinator at the i-th position within the
            // coordinators chain has a link to the j-th process,
            // increase the number of prior workers
            for (int j = 0; j < no_processes; j++) {
                if (topology[clusters_coordinators_order[i]][j] == 1) {
                    prior_workers_other_clusters++;
                }
            }
        }

        // calculate array chunk size and the remainder
        int array_chunk_size = array_length / total_workers;
        int array_chunk_remainder = array_length % total_workers;
        int last_index_parsed = 0;

        // coordinator sends the array data to its workers
        for (int i = 0; i < (int)subordinate_workers.size(); i++) {
            // calculate the current worker's chunk start and end
            int current_worker_start, current_worker_end;
            // if the number of prior workers is less than the remainder
            if (prior_workers_other_clusters + i < array_chunk_remainder) {
                // the formula for calculating the range, where the calculated
                // chunk = array_chunk_size + 1, so that the remainder is
                // divided equally between the first array_chunk_remainder
                // workers
                current_worker_start = (array_chunk_size + 1)
                        * (prior_workers_other_clusters + i);
                current_worker_end = (array_chunk_size + 1)
                        * (prior_workers_other_clusters + i + 1);
            // otherwise
            } else {
                // the formula for the other workers, where the chunk is equal
                // to array_chunk_size
                current_worker_start = array_chunk_remainder + array_chunk_size
                        * (prior_workers_other_clusters + i);
                current_worker_end = array_chunk_remainder + array_chunk_size
                        * (prior_workers_other_clusters + i + 1);
            }
            // the last index parsed will be the end of the chunk parsed
            // by the last worker
            last_index_parsed = current_worker_end;

            // send array length, array and the start and end of the chunk
            // to the current worker
            MPI_Send(&array_length, 1, MPI_INT, subordinate_workers[i],
                    0, MPI_COMM_WORLD);
            MPI_Send(calculus_array, array_length, MPI_INT,
                    subordinate_workers[i], 1, MPI_COMM_WORLD);
            MPI_Send(&current_worker_start, 1, MPI_INT, subordinate_workers[i],
                    2, MPI_COMM_WORLD);
            MPI_Send(&current_worker_end, 1, MPI_INT, subordinate_workers[i],
                    3, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << subordinate_workers[i] << ")\n";
        }

        // coordinator receives the array with the result calculus
        // from its workers
        for (int i = 0; i < (int)subordinate_workers.size(); i++) {
            // receive the array from the worker
            int* worker_calculus_array = (int*)malloc(sizeof(int)
                                        * array_length);
            MPI_Recv(worker_calculus_array, array_length, MPI_INT,
                subordinate_workers[i], 4, MPI_COMM_WORLD, &status);
            // calculate what range of the array the worker calculated
            int current_worker_start, current_worker_end;
            if (prior_workers_other_clusters + i < array_chunk_remainder) {
                current_worker_start = (array_chunk_size + 1)
                        * (prior_workers_other_clusters + i);
                current_worker_end = (array_chunk_size + 1)
                        * (prior_workers_other_clusters + i + 1);
            } else {
                current_worker_start = array_chunk_remainder + array_chunk_size
                        * (prior_workers_other_clusters + i);
                current_worker_end = array_chunk_remainder + array_chunk_size
                        * (prior_workers_other_clusters + i + 1);
            }
            // replace the coordinator's array within the chunk with the
            // worker's calculated values
            for (int i = current_worker_start; i < current_worker_end; i++) {
                calculus_array[i] = worker_calculus_array[i];
            }
        }

        // coordinator receives array from its coordinator child
        if (coordinator_child != -1) {
            // receive array
            int* child_calculus_array = (int*)malloc(sizeof(int)
                                        * array_length);
            MPI_Recv(child_calculus_array, array_length, MPI_INT,
                        coordinator_child, 5, MPI_COMM_WORLD, &status);
            // starting with the last parsed index, replace the values
            // in the current array with the values from the child
            for (int i = last_index_parsed; i < array_length; i++) {
                calculus_array[i] = child_calculus_array[i];
            }
        } 

        // coordinator sends array to its coordinator parent
        if (coordinator_parent != -1) {
            // send array
            MPI_Send(calculus_array, array_length, MPI_INT, coordinator_parent,
                    5, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << coordinator_parent << ")\n";
        }    
    }

    // if the process is not coordinator and it is NOT a worker of process 1
    // if the communication error is in the second case
    if (is_coordinator(rank) == false && (leader == 1
        && communication_error == 2) == 0) {
        // receive array length and array from leader
        MPI_Recv(&array_length, 1, MPI_INT, leader,
                    0, MPI_COMM_WORLD, &status);
        calculus_array = (int*)malloc(sizeof(int) * array_length);
        MPI_Recv(calculus_array, array_length, MPI_INT, leader,
                    1, MPI_COMM_WORLD, &status);
        // receive chunk start and end
        int current_worker_start, current_worker_end;
        MPI_Recv(&current_worker_start, 1, MPI_INT, leader,
                    2, MPI_COMM_WORLD, &status);
        MPI_Recv(&current_worker_end, 1, MPI_INT, leader,
                    3, MPI_COMM_WORLD, &status);
        // make calculus within the given range
        for (int i = current_worker_start; i < current_worker_end; i++) {
            calculus_array[i] *= 5;
        }
        // send the array to the parent
        MPI_Send(calculus_array, array_length, MPI_INT, leader,
                4, MPI_COMM_WORLD);
        cout << "M(" << rank << "," << leader << ")\n";
    }

    // if the rank is 0, it means it has the completed calculus array;
    // print the array
    if (rank == 0) { 
        cout << "Rezultat: ";
        for (int i = 0; i < array_length; i++) {
            cout << calculus_array[i] << " ";
        }
    }
}

#endif
