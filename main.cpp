#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

int main(int argc, char** argv) {
    std::cout << "Hello, World!" << std::endl;
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    /*vector<vector<int> > vect{ { 1, 2 },
                               { 3, 4 } };*/

    vector<int> map { 1, 2, 3, 4 };

    int mine;
    int current;

    if(rank == 0){
        for(int i=1; i< world_size; i++ ){
            current = map[i-1];
            MPI_Send(&current, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else{
        MPI_Recv(&current, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        mine = current;
    }

    cout <<"My rank: " << rank << ", my integer: " << mine << ", world_size: " << world_size << endl;

    MPI_Finalize();


}