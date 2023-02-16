// Profeanu Ioana, 333CA
// topology header file
#ifndef TOPOLOGY
#define TOPOLOGY

#include "tema3.h"

// function which for each coordinator, gets its neighbors
// (marked as parent and child) and then gets the order of the
// coordinators within the topology
void get_coordinator_neighbors(int rank, int communication_error, 
    int &coordinator_child, int &coordinator_parent,
    vector<int> &clusters_coordinators_order)
{
    // the topology chain order of the coordinators
    // is in the order 0, 3, 2, no matter the communication error
    clusters_coordinators_order.push_back(0);
    clusters_coordinators_order.push_back(3);
    clusters_coordinators_order.push_back(2);

    // the chain order starts from rank 0 (so rank 0 is at the top
    // of the chain); for each rank, get the parent and/or the child,
    // depending on the existing communication error
    if (rank == 3) {
        coordinator_child = 2;
        coordinator_parent = 0;
    }

    if (communication_error == 0) {
        if (rank == 0) {
            coordinator_child = 3;
        }
        if (rank == 1) {
            coordinator_parent = 2;
        }
        if (rank == 2) {
            coordinator_child = 1;
            coordinator_parent = 3;
        }
        // add 1 to the coordinators order
        clusters_coordinators_order.push_back(1);
    }

    if (communication_error == 1) {
        if (rank == 0) {
            coordinator_child = 3;
        }
        if (rank == 1) {
            coordinator_parent = 2;
        }
        if (rank == 2) {
            coordinator_child = 1;
            coordinator_parent = 3;
        }
        // add 1 to the coordinators order
        clusters_coordinators_order.push_back(1);
    }

    if (communication_error == 2) {
        if (rank == 0) {
            coordinator_child = 3;
        }
        if (rank == 2) {
            coordinator_parent = 3;
        }
    }
}

// function which returns the leader of a proccess
int worker_get_leader(int rank, vector<int> subordinate_workers,
                    MPI_Status status)
{
    int leader = -1;
    // each coordinator sends its rank to its subordinate workers
    if (is_coordinator(rank) == true) {
        for (int i = 0; i < (int)subordinate_workers.size(); i++) {
            MPI_Send(&rank, 1, MPI_INT, subordinate_workers[i],
                    0, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << subordinate_workers[i] << ")\n";
        }
    // each worker will receive a message from someone; that someone is
    // the leader sending its rank
    } else {
        MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE,
                0, MPI_COMM_WORLD, &status);
    }
    // return the rank of the leader
    return leader;
}

// function which returns the complete topology matrix
int** get_topology(int rank, vector<int> subordinate_workers,
    int coordinator_child, int coordinator_parent, int leader,
    int no_processes, MPI_Status status)
{
    // initialize own topology and the neighbor's topology matrix
    int **topology = (int **)malloc(sizeof(int*) * no_processes);
	int **neighbor_topology = (int **)malloc(sizeof(int*) * no_processes);
	
	for (size_t i = 0; i < (size_t)no_processes; i++) {
		topology[i] = (int *)calloc(sizeof(int), no_processes);
		neighbor_topology[i] = (int *)calloc(sizeof(int), no_processes);
	}

    // if the process is coordinator
    if (is_coordinator(rank) == true) {
        // add 1 in the topology matrix for each link with the workers
        for (int i = 0; i < (int)subordinate_workers.size(); i++) {
            topology[rank][subordinate_workers[i]] = 1;
            topology[subordinate_workers[i]][rank] = 1;
        }

        // receive the child's topology
        if (coordinator_child != -1) {
            for (int i = 0; i < no_processes; i++) {
                MPI_Recv(neighbor_topology[i], no_processes, MPI_INT,
                        coordinator_child, 0, MPI_COMM_WORLD, &status);
                for (int j = 0; j < no_processes; j++) {
                    if (topology[i][j] == 0) {
                        topology[i][j] = neighbor_topology[i][j];
                    }
                }
            }
        }

        // send own topology to parent
        if (coordinator_parent != -1) {
            for (int i = 0; i < no_processes; i++) {
                MPI_Send(topology[i], no_processes, MPI_INT,
                    coordinator_parent, 0, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << coordinator_parent << ")\n";
            }
        }

        // receive final topology from parent
        if (coordinator_parent != -1 && rank != 0) {
            for (int i = 0; i < no_processes; i++) {
                MPI_Recv(topology[i], no_processes, MPI_INT,
                    coordinator_parent, 1, MPI_COMM_WORLD, &status);
            }
        }

        // send final topology to child
        if (coordinator_child != -1) {
            for (int i = 0; i < no_processes; i++) {
                MPI_Send(topology[i], no_processes, MPI_INT,
                    coordinator_child, 1, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << coordinator_child << ")\n";
            }
        }

        // send final topology to workers
        for (int k = 0; k < (int)subordinate_workers.size(); k++) {
            for (int i = 0; i < no_processes; i++) {
                MPI_Send(topology[i], no_processes, MPI_INT,
                    subordinate_workers[k], 0, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << subordinate_workers[k] << ")\n";
            }
        }

    // if the process is worker
    } else {
        // worker retrieves final topology from coordinator
        for (int i = 0; i < no_processes; i++) {
            MPI_Recv(topology[i], no_processes, MPI_INT, leader,
                    0, MPI_COMM_WORLD, &status);
        }
    }
    // return the topology matrix
    return topology;
}

// function which prints the topology and gets the total number
// of existing workers within the topology
int print_topology(int rank, int no_processes, int** topology)
{
    int total_workers = 0;
    cout << rank << " -> ";
    // for each cluster leader
    for (int i = 0; i < NO_CLUSTERS; i++) {
        // get its workers and increase the number of workers
        vector<int> current_coordinator_workers;
        for (int j = 0; j < no_processes; j++) {
            if (topology[i][j] == 1) {
                current_coordinator_workers.push_back(j);
                total_workers++;
            }
        }

        // print the topology of the current leader's cluster
        if (current_coordinator_workers.size() > 0) {
            cout << i << ":";
            int check_if_first = true;
            for (int j = 0;
                j < (int)current_coordinator_workers.size(); j++) {
                if (check_if_first == true) {
                    cout  << current_coordinator_workers[j];
                    check_if_first = false;
                } else {
                    cout << "," << current_coordinator_workers[j];
                }
            }
            cout << " ";
        }
    }
    cout << endl;

    // return the number of total workers within the topology
    return total_workers;
}

#endif
