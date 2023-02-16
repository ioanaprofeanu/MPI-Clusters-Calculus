// Profeanu Ioana, 333CA
// header file
#ifndef TEMA3
#define TEMA3

#include <mpi.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

#define NO_CLUSTERS 4

// function for checking if a process is coordinator or not,
// based on its rank
bool is_coordinator(int rank) {
    if (rank == 0 || rank == 1 || rank == 2 || rank == 3) {
        return true;
    }
    return false;
}

// function which if the process is leader, reads its specific file
void read_file(vector<int> &subordinate_workers, int rank)
{
    // get the file name correspondent to the rank
    string file_name;
    if (rank == 0) {
        file_name = "cluster0.txt";
    }
    if (rank == 1) {
        file_name = "cluster1.txt";
    }
    if (rank == 2) {
        file_name = "cluster2.txt";
    }
    if (rank == 3) {
        file_name = "cluster3.txt";
    }

    // read the workers from the file and add them
    // to coordinator's workers vector
    ifstream fin(file_name);
    int number_of_workers;
    fin >> number_of_workers;
    for (int i = 0; i < number_of_workers; i++) {
        int worker;
        fin >> worker;
        subordinate_workers.push_back(worker);
    }
}

#endif
