#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <math.h>

using namespace std;

vector<vector<int>> get_input(string file_name){ // Read the input from the text file
    vector<vector<int>> map;
    ifstream in_file;
    in_file.open(file_name);
    if (!in_file) {
        cerr << "Unable to open the input file";
        exit(1);
    }
    int current_int;
    for(int i=0; i<360; i++){
        vector<int> current_vec;
        for(int j=0; j<360; j++){
            in_file >> current_int;
            current_vec.push_back(current_int);
        }
        map.push_back(current_vec);
    }

    in_file.close();
    return map;
}

void print_result(int n, vector<vector<int>> map, string file_name){ // Print the resulting map to file
    ofstream out_file;
    out_file.open (file_name);

    int count = 0;
    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            out_file << map[i][j] << " ";
            count += map[i][j];
        }
        out_file << endl;
    }
    out_file.close();

}

vector<vector<int>> base_submap(int c){ // Create a empty map for processes
    vector<vector<int>> base_submap;
    int sqrt_c = sqrt(c);
    int count = 360 / sqrt_c; // number of columns/rows
    for(int i=0; i<count+2; i++){ // + 2 for paddings
        vector<int> row;
        for(int j=0; j<count+2; j++){
            row.push_back(9); // 9 is a filler integer
        }
        base_submap.push_back(row);
    }
    return base_submap;
}

int calculate_target(int mode, int sqrt_c, int c, int rank, int row_index = -1, int column_index = -1){ // Calculate processes' communication peers' rank

    int target;

    if(mode == 1){ // Upper left corner neighbor
        if (column_index == 0) {
            target = rank - 1;
            if (target == 0) {
                target = c;
            }
        } else {
            target = (rank - sqrt_c - 1);
            if (target <= 0) {
                target += c;
            }
        }
    }
    if(mode == 2){ // Upper neighbor
        target = (rank - sqrt_c);
        if(target > c ){
            target -= c;
        }
        else if(target <= 0){
            target += c;
        }
    }
    if(mode == 3){ // Upper right corner neighbor
        if(column_index == sqrt_c - 1){
            target = (rank - (2*sqrt_c) + 1);
            if(target < 0){
                target = c - sqrt_c + 1;
            }
        }
        else{
            target = (rank - sqrt_c + 1);
            if(target <= 0){
                target = target + c;
            }
        }
    }
    if(mode == 4){ // Left neighbor
        target = (rank - 1);

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }
    }
    if(mode == 5 ){
        target = (rank + 1); // Calculate right neighbor's rank

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }
    }
    if(mode == 6){ // Calculate lower left neighbor's rank
        if(column_index == 0){
            target = (rank + (2*sqrt_c) - 1);
            if(target > c){
                target = sqrt_c;
            }
        }
        else{
            target = (rank + sqrt_c - 1);
            if(target > c){
                target = target - c;
            }
        }
    }
    if( mode == 7){ // Calculate lower neighbor's rank
        target = (rank + sqrt_c); // Calculate bottom process' rank

        if (target > c) {
            target -= c;
        } else if (target <= 0) {
            target += c;
        }
    }
    if( mode == 8){ // Calculate lower right corner neighbor's rank
        if(column_index == sqrt_c - 1){
            target = rank + 1;
            if(target > c){
                target = 1;
            }
        }
        else{
            target = (rank + sqrt_c + 1);
            if(target > c){
                target = target - c;
            }
        }
    }


    return target;
}


int main(int argc, char** argv) {

    if(argc != 4){
        cout << "Number of parameters should be 3." <<endl;
        return 1;
    }
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get rank
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Get world size


    vector<vector<int>> map;
    int row_index;
    int column_index;

    int temp; // Used as temp variable to move integers
    int c = world_size - 1; // number of worker processes
    int sqrt_c = sqrt(c); // Number of columns/rows
    int offset = 360 / sqrt_c; // Index offset for loops, also number of columns/rows
    vector<int> flat_sub_map; // Flatted process map for sending map with MPI_Send
    vector<int> send_map; // Map to be sended to master process after all computations are done.
    vector<int> message; // Array to be shared
    int corner; // Corner integer to be shared
    int iteration = stoi(argv[3]); // number of iteration requested

    if(rank == 0){ // Master process, distributes maps

        map = get_input(argv[1]); // Read from file
        for(int r=1; r< world_size; r++){ // Iterate through ranks and distribute their sub maps.
            row_index = (r-1) / sqrt_c;
            column_index = r - (sqrt_c*row_index) - 1;


            for(int i=row_index*offset; i<row_index*offset+offset; i++){

                for(int j=column_index*offset; j<column_index*offset+offset; j++){

                    flat_sub_map.push_back(map[i][j]);

                }
            }
            MPI_Send(&flat_sub_map[0], offset*offset, MPI_INT, r, 0, MPI_COMM_WORLD);
            flat_sub_map.clear();
        }
    }

    else if(rank % 2 == 1 ){ // Odd numbered process, first odd ones send information
        flat_sub_map.resize(offset*offset);
        MPI_Recv(&flat_sub_map[0], offset*offset, MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        vector<vector<int>> my_submap = base_submap(c);
        int iterate = 0; // iterate through flat_sub_map array
        for(int i=1; i<offset+1; i++){
            for (int j = 1; j < offset+1; j++) {
                my_submap[i][j] = flat_sub_map.at(iterate);
                iterate++;
            }
        }

        int target; // Rank of peer process
        row_index = (rank-1) / sqrt_c; // current row index
        column_index = rank - (sqrt_c*row_index) - 1; // current column index
        int iterate_received = 0; // Iterate received message

        for(int i=0; i<iteration; i++){ // Main loop, does the iterations.
        // SEND PART

        if(row_index % 2 == 1 ){ // First odd rows send bottom and upper parts

            // Send bottom part
            for(int i=1; i<offset+1; i++){ // bottom part of process
                message.push_back(my_submap[offset][i]);
            }
            target = calculate_target(7, sqrt_c, c, rank, row_index, column_index); // Get bottom neighbor
            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);
            message.clear();

            // Send upper part

            for(int i=1; i<offset+1; i++){
                message.push_back(my_submap[1][i]);
            }
            target = calculate_target(2, sqrt_c, c, rank, row_index, column_index);
            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);
            message.clear();

            // Receive upper part ( Peer's bottom part )
            message.resize(offset);
            target = calculate_target(2, sqrt_c, c, rank, row_index, column_index);
            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for(int i=1; i<offset+1; i++){
                my_submap[0][i] = message[iterate_received];
                iterate_received++;
            }

            iterate_received = 0;

            message.clear();

            // Receive bottom part ( Peer's upper part )
            target = calculate_target(7, sqrt_c, c, rank, row_index, column_index); // Get bottom neighbor
            message.resize(offset);
            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(int i=1; i<offset+1; i++){
                my_submap[offset+1][i] = message[iterate_received];
                iterate_received++;
            }

            iterate_received = 0;

            message.clear();

        }
        else{

            // Receive upper part ( Peer's bottom part )

            message.resize(offset);
            target = calculate_target(2, sqrt_c, c, rank, row_index, column_index);
            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for(int i=1; i<offset+1; i++){
                my_submap[0][i] = message[iterate_received];
                iterate_received++;
            }

            iterate_received = 0;

            message.clear();

            // Receive bottom part ( Peer's upper part )
            target = calculate_target(7, sqrt_c, c, rank, row_index, column_index); // Get bottom neighbor
            message.resize(offset);
            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(int i=1; i<offset+1; i++){
                my_submap[offset+1][i] = message[iterate_received];
                iterate_received++;
            }

            iterate_received = 0;

            message.clear();

            // Send bottom part
            for(int i=1; i<offset+1; i++){ // bottom part of process
                message.push_back(my_submap[offset][i]);
            }
            target = calculate_target(7, sqrt_c, c, rank, row_index, column_index); // Get bottom neighbor
            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();

            // Send upper part

            for(int i=1; i<offset+1; i++){
                message.push_back(my_submap[1][i]);
            }
            target = calculate_target(2, sqrt_c, c, rank, row_index, column_index);
            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();


        }

        // LEFT-RIGHT SEND PART

        // Send right part

        for(int i=1; i<offset+1; i++){ // first element is top element
            message.push_back(my_submap[i][offset]);
        }
        target = calculate_target(5, sqrt_c, c, rank, row_index, column_index); // Get right neighbor's rank
        MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

        message.clear();

        // Send left part
        for(int i=1; i<offset+1; i++){
            message.push_back(my_submap[i][1]);
        }
        target = calculate_target(4, sqrt_c, c, rank, row_index, column_index);
        MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

        message.clear();

        // LEFT-RIGHT RECEIVE PART
        // Receive left part ( Peer's right part )
        message.resize(offset);
        calculate_target(4, sqrt_c, c, rank, row_index, column_index);
        MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i=1; i<offset+1; i++){
            my_submap[i][0] = message[iterate_received];
            iterate_received++;
        }
        iterate_received = 0;
        message.clear();

        // Receive right part ( Peer's left part )
        message.resize(offset);
        target = calculate_target(5, sqrt_c, c, rank, row_index, column_index); // Get right neighbor's rank
        MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for(int i=1; i<offset+1; i++){
            my_submap[i][offset+1] = message[iterate_received];
            iterate_received++;
        }

        iterate_received = 0;
        message.clear();

        // SEND CORNERS PART
        // Send upper left corner
        corner = my_submap[1][1];
        target = calculate_target(1, sqrt_c, c, rank, row_index, column_index); // Get upper left neighbor
        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

        // Send upper right corner
        corner = my_submap[1][offset];
        target = calculate_target(3, sqrt_c, c, rank, row_index, column_index);
        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

        // Send lower left corner
        corner = my_submap[offset][1];
        target = calculate_target(6, sqrt_c, c, rank, row_index, column_index); // Get lower left neighbor
        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

        // Send lower right corner
        corner = my_submap[offset][offset];
        target = calculate_target(8, sqrt_c, c, rank, row_index, column_index); // Get lower right neighbor
        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

        // RECEIVE CORNERS PART
        // Receive lower right corner
        target = calculate_target(8, sqrt_c, c, rank, row_index, column_index); // Get lower right neighbor
        MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        my_submap[offset+1][offset+1] = corner;

        // Receive lower left corner
        target = calculate_target(6, sqrt_c, c, rank, row_index, column_index); // Get lower left neighbor
        MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        my_submap[offset+1][0] = corner;

        // Receive upper right corner
        target = calculate_target(3, sqrt_c, c, rank, row_index, column_index);
        MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        my_submap[0][offset+1] = corner;

        // Receive upper left corner
        target = calculate_target(1, sqrt_c, c, rank, row_index, column_index); // Get upper left neighbor
        MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        my_submap[0][0] = corner;
        vector<vector<int>> temp_submap = my_submap;
        for(int row=1; row<offset+1; row++){
            for(int column=1; column<offset+1; column++){
                int count; // count number of 1's
                count = my_submap[row-1][column-1] +  my_submap[row-1][column] +  my_submap[row-1][column+1] +  my_submap[row][column-1]
                        + my_submap[row][column+1] + my_submap[row+1][column-1] + my_submap[row+1][column] + my_submap[row+1][column+1];

                if(count < 2 || count > 3){
                    temp_submap[row][column] = 0;
                }
                else if(count == 3){
                    temp_submap[row][column] = 1;

                }
            }
        }
        my_submap = temp_submap;
        }

        for(int i=1; i<offset+1; i++){ // Populate send map to be sended to master process
            for(int j=1; j<offset+1; j++){
                send_map.push_back(my_submap[i][j]);

            }
        }
        MPI_Send(&send_map[0], offset*offset, MPI_INT, 0, 0, MPI_COMM_WORLD); // Sen map to the master process
        send_map.clear();
    }

    else if(rank % 2 == 0 ){ // Even numbered processes receive first.
        flat_sub_map.resize(offset*offset);
        MPI_Recv(&flat_sub_map[0], offset*offset, MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        vector<vector<int>> my_submap = base_submap(c);
        int iterate = 0; // iterate through flat_sub_map array
        for(int i=1; i<offset+1; i++){
            for (int j = 1; j < offset+1; j++) {
                my_submap[i][j] = flat_sub_map.at(iterate);
                iterate++;

            }
        }

        int target; // Rank of peer process
        row_index = (rank-1) / sqrt_c; // current row index
        column_index = rank - (sqrt_c*row_index) - 1; // current column index
        int iterate_received = 0; // Iterate received message

        for(int i=0; i<iteration; i++) { // Main loop, does the iterations

            if (row_index % 2 == 1) { // First odd rows send bottom and upper parts

                // Send bottom part
                for (int i = 1; i < offset + 1; i++) { // bottom part of process
                    message.push_back(my_submap[offset][i]);
                }
                target = calculate_target(7, sqrt_c, c, rank, row_index, column_index); // Get bottom neighbor
                MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

                message.clear();

                // Send upper part

                for (int i = 1; i < offset + 1; i++) {
                    message.push_back(my_submap[1][i]);
                }
                target = calculate_target(2, sqrt_c, c, rank, row_index, column_index);
                MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);
                message.clear();

                // Receive upper part ( Peer's bottom part )

                message.resize(offset);
                target = calculate_target(2, sqrt_c, c, rank, row_index, column_index);
                MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int i = 1; i < offset + 1; i++) {
                    my_submap[0][i] = message[iterate_received];
                    iterate_received++;
                }

                iterate_received = 0;

                message.clear();

                // Receive bottom part ( Peer's upper part )
                message.resize(offset);
                target = calculate_target(7, sqrt_c, c, rank, row_index, column_index); // Get bottom neighbor
                MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int i = 1; i < offset + 1; i++) {
                    my_submap[offset + 1][i] = message[iterate_received];
                    iterate_received++;
                }
                iterate_received = 0;
                message.clear();

            } else {

                // Receive upper part ( Peer's bottom part )
                message.resize(offset);
                target = calculate_target(2, sqrt_c, c, rank, row_index, column_index);
                MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int i = 1; i < offset + 1; i++) {
                    my_submap[0][i] = message[iterate_received];
                    iterate_received++;
                }
                iterate_received = 0;
                message.clear();

                // Receive bottom part ( Peer's upper part )
                message.resize(offset);
                target = calculate_target(7, sqrt_c, c, rank, row_index, column_index); // Get bottom neighbor
                MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                for (int i = 1; i < offset + 1; i++) {
                    my_submap[offset + 1][i] = message[iterate_received];
                    iterate_received++;
                }

                iterate_received = 0;
                message.clear();

                // Send bottom part
                for (int i = 1; i < offset + 1; i++) { // bottom part of process
                    message.push_back(my_submap[offset][i]);
                }
                target = calculate_target(7, sqrt_c, c, rank, row_index, column_index); // Get bottom neighbor
                MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

                message.clear();

                // Send upper part
                for (int i = 1; i < offset + 1; i++) {
                    message.push_back(my_submap[1][i]);
                }
                target = calculate_target(2, sqrt_c, c, rank, row_index, column_index);
                MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

                message.clear();

            }

            // LEFFT-RIGHT RECEIVE PART
            // Receive left part ( Peer's right part )

            message.resize(offset);
            target = calculate_target(4, sqrt_c, c, rank, row_index, column_index);
            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int i = 1; i < offset + 1; i++) {
                my_submap[i][0] = message[iterate_received];
                iterate_received++;
            }

            iterate_received = 0;
            message.clear();

            // Receive right part ( Peer's left part )
            message.resize(offset);
            target = calculate_target(5, sqrt_c, c, rank, row_index, column_index); // Get right neighbor's rank
            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int i = 1; i < offset + 1; i++) {
                my_submap[i][offset + 1] = message[iterate_received];
                iterate_received++;
            }
            iterate_received = 0;
            message.clear();

            // LEFFT-RIGHT SEND PART
            // Send right part

            for (int i = 1; i < offset + 1; i++) { // first element is top element
                message.push_back(my_submap[i][offset]);
            }

            target = (rank + 1); // Calculate right process' rank
            target = calculate_target(5, sqrt_c, c, rank, row_index, column_index); // Get right neighbor's rank
            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);
            message.clear();

            // Send left part
            for (int i = 1; i < offset + 1; i++) {
                message.push_back(my_submap[i][1]);
            }
            target = calculate_target(4, sqrt_c, c, rank, row_index, column_index);
            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);
            message.clear();
            // RECEIVE CORNERS PART
            // Receive lower right corner
            target = calculate_target(8, sqrt_c, c, rank, row_index, column_index); // Get lower right neighbor
            MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            my_submap[offset + 1][offset + 1] = corner;

            // Receive lower left corner
            target = calculate_target(6, sqrt_c, c, rank, row_index, column_index); // Get lower left neighbor
            MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            my_submap[offset + 1][0] = corner;

            // Receive upper right corner
            target = calculate_target(3, sqrt_c, c, rank, row_index, column_index);
            MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            my_submap[0][offset + 1] = corner;

            // Receive upper left corner
            target = calculate_target(1, sqrt_c, c, rank, row_index, column_index); // Get upper left neighbor
            MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            my_submap[0][0] = corner;

            // SEND CORNERS PART
            // Send upper left corner
            corner = my_submap[1][1];
            target = calculate_target(1, sqrt_c, c, rank, row_index, column_index); // Get upper left neighbor
            MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

            // Send upper right corner
            corner = my_submap[1][offset];
            target = calculate_target(3, sqrt_c, c, rank, row_index, column_index);
            MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

            // Send lower left corner
            corner = my_submap[offset][1];
            target = calculate_target(6, sqrt_c, c, rank, row_index, column_index); // Get lower left neighbor
            MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

            // Send lower right corner
            corner = my_submap[offset][offset];
            target = calculate_target(8, sqrt_c, c, rank, row_index, column_index); // Get lower right neighbor
            MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

            vector<vector<int>> temp_submap = my_submap;
            for(int row=1; row<offset+1; row++){
                for(int column=1; column<offset+1; column++){
                    int count= 0; // count number of 1's
                    count = count +  my_submap[row-1][column-1] +  my_submap[row-1][column] +  my_submap[row-1][column+1] +  my_submap[row][column-1]
                            + my_submap[row][column+1] + my_submap[row+1][column-1] + my_submap[row+1][column] + my_submap[row+1][column+1];

                    if(count < 2 || count > 3){
                        temp_submap[row][column] = 0;
                    }
                    else if(count == 3){
                        temp_submap[row][column] = 1;
                    }
                }
            }

            my_submap = temp_submap;
        }

        for(int i=1; i<offset+1; i++){ // Populate send map to be sended to master process
            for(int j=1; j<offset+1; j++){
                send_map.push_back(my_submap[i][j]);
            }
        }
        MPI_Send(&send_map[0], offset*offset, MPI_INT, 0, 0, MPI_COMM_WORLD); // Send map to master process
        send_map.clear();
    }
    if(rank == 0){

        for(int r=1; r< world_size; r++){ // Iterate through ranks and collect their sub maps.
            row_index = (r-1) / sqrt_c;
            column_index = r - (sqrt_c*row_index) - 1;
            send_map.resize(offset*offset);
            MPI_Recv(&send_map[0], offset*offset, MPI_INT, r, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            int iterate = 0;
            for(int i=row_index*offset; i<row_index*offset+offset; i++){
                for(int j=column_index*offset; j<column_index*offset+offset; j++){
                    map[i][j] = send_map[iterate];
                    iterate++;
                }
            }
        }
        print_result(360, map, argv[2]);
    }
    MPI_Finalize();

}