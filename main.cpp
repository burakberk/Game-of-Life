#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <math.h>

using namespace std;

vector<vector<int>> get_input(string file_name){
    vector<vector<int>> map;
    ifstream in_file;
    in_file.open(file_name);
    if (!in_file) {
        cerr << "Unable to open the input file";
        exit(1);
    }
    int current_int;
    for(int i=0; i<2; i++){
        vector<int> current_vec;
        for(int j=0; j<2; j++){
            in_file >> current_int;
            current_vec.push_back(current_int);
            /*cout << "INPUT STREAM FOUND 1" << endl;*/
        }
        map.push_back(current_vec);
    }

    in_file.close();

    return map;

}

void print_result(int n, vector<vector<int>> map){
    ofstream out_file;
    out_file.open ("output.txt");


    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            out_file << map[i][j] << " ";
        }
        out_file << endl;
    }

    out_file.close();

}

vector<int> flat_map(vector<vector<int>> map){
    vector<int> flatted_map;
    for (int i = 0; i < map.size(); i++) {
        for (int j = 0; j < map[0].size(); j++) {
            flatted_map.push_back(map[i][j]);
        }
    }

    return flatted_map;
}

vector<vector<int>> unpack_map(int n, vector<int> map){
    vector<vector<int>> unpacked_map;
    int count = 0;
    for (int i = 0; i < n; i++) {
        vector<int> current_vec;
        for (int j = 0; j < n; j++) {
            current_vec.push_back(map.at(count));
            count++;
        }
        unpacked_map.push_back(current_vec);
    }

    return unpacked_map;
}

vector<vector<int>> base_submap(int c){
    vector<vector<int>> base_submap;
    int sqrt_c = sqrt(c);
    int count = 2 / sqrt_c; // number of columns/rows
    for(int i=0; i<count+2; i++){ // + 2 for paddings
        vector<int> row;
        for(int j=0; j<count+2; j++){
            row.push_back(9); // 9 is a filler integer
        }
        base_submap.push_back(row);
    }
    return base_submap;
}
int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    vector<vector<int> > vect{ { 1, 2 },
                               { 3, 4 } };

    vector<int> map2{1,2,3,4};

    vector<vector<int>> map;

    map = get_input("my_input.txt");
    print_result(2, map);

    int row_index;
    int column_index;

    int mine;
    int temp; // Used as temp variable to move integers
    int c = 4; // number of worker processes
    int sqrt_c = sqrt(c);
    int offset = 2 / sqrt_c; // Index offset for loops, also number of columns/rows
    vector<int> flat_sub_map;
    vector<int> message; // Array to be shared
    int corner; // Corner integer to be shared

    if(rank == 0){




        for(int r=1; r< world_size; r++){ // Iterate through ranks and distribute their sub maps.
            row_index = (r-1) / sqrt_c;
            column_index = r - (sqrt_c*row_index) - 1;
            /*vector<int> flat_sub_map;*/

            for(int i=row_index*offset; i<row_index*offset+offset; i++){

                for(int j=column_index*offset; j<column_index*offset+offset; j++){

                    flat_sub_map.push_back(map[i][j]);


                }
            }
            /*cout << flat_sub_map.size() << endl;*/

            MPI_Send(&flat_sub_map[0], offset*offset, MPI_INT, r, 0, MPI_COMM_WORLD);
            flat_sub_map.clear();

        }



    }

    else if(rank % 2 == 1 ){
        flat_sub_map.resize(14400);
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


        // SEND PART

        if(row_index % 2 == 1 ){ // First odd rows send bottom and upper parts

            // Send bottom part
            for(int i=1; i<offset+1; i++){ // bottom part of process
                message.push_back(my_submap[offset][i]);
            }

            target = (rank + sqrt_c); // Calculate bottom process' rank
            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();

            // Send upper part

            for(int i=1; i<offset+1; i++){
                message.push_back(my_submap[1][i]);
            }

            target = (rank - sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();

            // Receive upper part ( Peer's bottom part )

            message.resize(offset);

            target = (rank - sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for(int i=1; i<offset+1; i++){
                my_submap[0][i] = message[iterate_received];
                iterate_received++;
            }

            iterate_received = 0;

            message.clear();

            // Receive bottom part ( Peer's upper part )

            target = (rank + sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

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

            target = (rank - sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for(int i=1; i<offset+1; i++){
                my_submap[0][i] = message[iterate_received];
                iterate_received++;
            }

            iterate_received = 0;

            message.clear();

            // Receive bottom part ( Peer's upper part )

            target = (rank + sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

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

            target = (rank + sqrt_c); // Calculate bottom process' rank
            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();

            // Send upper part

            for(int i=1; i<offset+1; i++){
                message.push_back(my_submap[1][i]);
            }

            target = (rank - sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();


        }

        // LEFT-RIGHT SEND PART

        // Send right part

        for(int i=1; i<offset+1; i++){ // first element is top element
            message.push_back(my_submap[i][offset]);
        }

        target = (rank + 1); // Calculate right process' rank

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }

        MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

        message.clear();



        // Send left part

        for(int i=1; i<offset+1; i++){
            message.push_back(my_submap[i][1]);
        }

        target = (rank - 1);

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }

        MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

        message.clear();

        // LEFT-RIGHT RECEIVE PART


        // Receive left part ( Peer's right part )

        message.resize(offset);

        target = (rank - 1); // Calculate left process' rank

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }

        MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for(int i=1; i<offset+1; i++){
            my_submap[i][0] = message[iterate_received];
            iterate_received++;
        }

        iterate_received = 0;

        message.clear();




        // Receive right part ( Peer's left part )

        message.resize(offset);

        target = (rank + 1); // Calculate right process' rank

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }

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

        if(column_index == 0){
            target = rank - 1;
            if(target == 0){
                target = c;
            }
        }
        else{
            target = (rank - sqrt_c - 1);
            if(target <= 0){
                target += c;
            }
        }

        cout << "rank: " << rank << ", target: " << target << ", column index: " << column_index << endl;
        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

/*        // Send upper right corner

        corner = my_submap[1][offset];

        if(column_index == sqrt_c - 1){
            target = (rank - (2*sqrt_c) + 1);
            if(target < 0){
                target = c - sqrt_c + 1;
            }
        }
        else{
            target = (rank - sqrt_c + 1);
            if(target < 0){
                target = c- sqrt_c;
            }
        }

        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

        // Send lower left corner

        corner = my_submap[offset][1];

        if(column_index == 0){
            target = (rank + (2*sqrt_c) - 1);
            if(target > 0){
                target = sqrt_c;
            }
        }
        else{
            target = (rank + sqrt_c - 1);
            if(target > 0){
                target = target - c;
            }
        }

        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

        // Send lower right corner

        corner = my_submap[offset][offset];


        if(column_index == sqrt_c - 1){
            target = target + 1;
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

        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);


        cout << "rank: " << rank << ", sub_map_size: " << my_submap.size() << endl;*/




    }

    else if(rank % 2 == 0 ){
        flat_sub_map.resize(14400);
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

        if(row_index % 2 == 1 ){ // First odd rows send bottom and upper parts

            // Send bottom part
            for(int i=1; i<offset+1; i++){ // bottom part of process
                message.push_back(my_submap[offset][i]);
            }

            target = (rank + sqrt_c); // Calculate bottom process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();

            // Send upper part

            for(int i=1; i<offset+1; i++){
                message.push_back(my_submap[1][i]);
            }

            target = (rank - sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();

            // Receive upper part ( Peer's bottom part )

            message.resize(offset);

            target = (rank - sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for(int i=1; i<offset+1; i++){
                my_submap[0][i] = message[iterate_received];
                iterate_received++;
            }

            iterate_received = 0;

            message.clear();

            // Receive bottom part ( Peer's upper part )

            message.resize(offset);

            target = (rank + sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

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

            target = (rank - sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for(int i=1; i<offset+1; i++){
                my_submap[0][i] = message[iterate_received];
                iterate_received++;
            }

            iterate_received = 0;

            message.clear();

            // Receive bottom part ( Peer's upper part )

            message.resize(offset);

            target = (rank + sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

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

            target = (rank + sqrt_c); // Calculate bottom process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();

            // Send upper part

            for(int i=1; i<offset+1; i++){
                message.push_back(my_submap[1][i]);
            }

            target = (rank - sqrt_c); // Calculate upper process' rank

            if(target > c ){
                target -= c;
            }
            else if(target <= 0){
                target += c;
            }

            MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

            message.clear();


        }


        // LEFFT-RIGHT RECEIVE PART


        // Receive left part ( Peer's right part )

        message.resize(offset);

        target = (rank - 1); // Calculate left process' rank

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }

        MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for(int i=1; i<offset+1; i++){
            my_submap[i][0] = message[iterate_received];
            iterate_received++;
        }

        iterate_received = 0;

        message.clear();


        // Receive right part ( Peer's left part )

        message.resize(offset);

        target = (rank + 1); // Calculate right process' rank

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }

        MPI_Recv(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for(int i=1; i<offset+1; i++){
            my_submap[i][offset+1] = message[iterate_received];
            iterate_received++;
        }

        iterate_received = 0;

        message.clear();


        // LEFFT-RIGHT SEND PART


        // Send right part

        for(int i=1; i<offset+1; i++){ // first element is top element
            message.push_back(my_submap[i][offset]);
        }

        target = (rank + 1); // Calculate right process' rank

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }

        MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

        message.clear();


        // Send left part

        for(int i=1; i<offset+1; i++){
            message.push_back(my_submap[i][1]);
        }

        target = (rank - 1);

        if(target > (row_index+1)*sqrt_c ){
            target -= sqrt_c;
        }
        else if(target <= (row_index)*sqrt_c){
            target += sqrt_c;
        }

        MPI_Send(&message[0], offset, MPI_INT, target, 0, MPI_COMM_WORLD);

        message.clear();



        /*cout << "rank: " << rank << ", sub_map_size: " << my_submap.size() << endl;*/
        if(rank == 4){
            print_result(3, my_submap);
        }

        // RECEIVE CORNERS PART

        // Receive lower right corner
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

        cout << "rank: " << rank << ", target: " << target << ", column index: " << column_index << endl;

        MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        my_submap[offset+1][offset+1] = corner;
        

        /*// Receive lower left corner

        if(column_index == 0){
            target = (rank + (2*sqrt_c) - 1);
            if(target > 0){
                target = sqrt_c;
            }
        }
        else{
            target = (rank + sqrt_c - 1);
            if(target > 0){
                target = target - c;
            }
        }

        MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        my_submap[offset+1][0] = corner;

        // Receive upper right corner

        if(column_index == sqrt_c - 1){
            target = (rank - (2*sqrt_c) + 1);
            if(target < 0){
                target = c - sqrt_c + 1;
            }
        }
        else{
            target = (rank - sqrt_c + 1);
            if(target < 0){
                target = c- sqrt_c;
            }
        }

        MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        my_submap[0][offset+1] = corner;

        // Receive upper left corner

        if(column_index == 0){
            target = rank - 1;
            if(target == 0){
                target = c;
            }
        }
        else{
            target = (rank - sqrt_c - 1);
            if(target <= 0){
                target += c;
            }
        }


        MPI_Recv(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        my_submap[0][0] = corner;*/




        /*// SEND CORNERS PART

        // Send upper left corner

        corner = my_submap[1][1];

        target = (rank - row_index - 1);

        if(target > c ){
            target -= c;
        }
        else if(target <= 0){
            target += c;
        }

        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

        // Send upper right corner

        corner = my_submap[1][offset];

        target = (rank - row_index + 1);

        if(target > c ){
            target -= c;
        }
        else if(target <= 0){
            target += c;
        }

        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

        // Send lower left corner

        corner = my_submap[offset][1];

        target = (rank + row_index - 1);

        if(target > c ){
            target -= c;
        }
        else if(target <= 0){
            target += c;
        }

        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);

        // Send lower right corner

        corner = my_submap[offset][offset];

        target = (rank + row_index + 1);

        if(target > c ){
            target -= c;
        }
        else if(target <= 0){
            target += c;
        }

        MPI_Send(&corner, 1, MPI_INT, target, 0, MPI_COMM_WORLD);*/



    }





    MPI_Finalize();


}