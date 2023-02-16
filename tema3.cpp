// Profeanu Ioana, 333CA
// main source file

#include "tema3.h"
#include "topology.h"
#include "calculus.h"

int main(int argc, char * argv[])
{
    int no_processes, rank;
    int communication_error = stoi(argv[2]);

    // initialize mpi
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &no_processes);

    // if the process is coordinator, read its specific file
    vector<int> subordinate_workers;
    if (is_coordinator(rank) == true) {
        read_file(subordinate_workers, rank);
    }

    // get the order of the leaders according to the communication
    // error and get each leader's order within the leaders chain
    int coordinator_child = -1, coordinator_parent = -1;
    vector<int> clusters_coordinators_order;
    get_coordinator_neighbors(rank, communication_error,
                        coordinator_child, coordinator_parent,
                        clusters_coordinators_order);

    // get the leader of the process
    int leader = worker_get_leader(rank, subordinate_workers, status);

    MPI_Barrier(MPI_COMM_WORLD);

    // get the topology
    int** topology = get_topology(rank, subordinate_workers, coordinator_child,
            coordinator_parent, leader, no_processes, status);
    // print the topology and get the total number of workers within it
    int total_workers = print_topology(rank, no_processes, topology);

    MPI_Barrier(MPI_COMM_WORLD);

    int array_length = 0;
    int *calculus_array;
    // the process with rank 0 generates the calculus array
    if (rank == 0) {
        array_length = stoi(argv[1]);
        calculus_array = (int*)malloc(sizeof(int) * array_length);
        for (int k = 0; k < array_length; k++) {
            calculus_array[k] = array_length - k - 1;
        }
    }

    // make the calculus
    make_calculus(rank, leader, no_processes, array_length,
        communication_error, status, total_workers, coordinator_parent,
        coordinator_child, calculus_array, subordinate_workers,
        clusters_coordinators_order, topology);

    MPI_Finalize();

    return 0;
}
