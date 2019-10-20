#include "Library.h"
#include "LSH.h"
#include "LSH_Functions.h"
#include "BHC.h"
#include "BHC_Functions.h"
#include "Helper_Functions.h"
#include "HashTable.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "We need input_file AND query_file!" << endl;
        return -1;
    }
    /* variable declaration | k = 4 default value */
    int error_code, k = 4, dim = 3, M = 10, probes = 2;                       // k is the number of hi concatenated to form g - dim is number of hypercube's vertices
    /* vectors for the data and query points */
    vector<vector<int>> dataset;
    vector<vector<int>> searchset;

    /* read data set and query set and load them in vectors */
    error_code = Read_point_files(&dataset, &searchset, argv[1], argv[2]);
    if (error_code == -1) return -1;

    vector<int> TrueDistances;
    vector<double> TrueTimes;
    /* do brute force to find actual NNs */
//#ifdef BRUTE_FORCE
    brute_force(&dataset, &searchset, &TrueDistances, &TrueTimes);
//#endif

//    /* amplified hash for dataset*/
    vector<vector<int>> data_amplified_g;
//    /* amplified hash for searchset */
    vector<vector<int>> query_amplified_g;
//    /* results */
    vector<vector<vector<vector<int>>>> ANN;

    int Metric = 1; // default value for Manhattan Distance
    BHC(dataset, searchset, k, dim, M, probes, data_amplified_g, query_amplified_g, &ANN);

    int distance = 0;
    int *min_distance = new int [searchset.size()];
    int *nearest_neighbor = new int [searchset.size()];
    double *time = new double [searchset.size()];
    double max_af = 0.0;
    double average_af = 0.0;
    double curr_fraction = 0.0;
    double average_time = 0.0;

    /* initialize arrays */
    for (int i = 0; i < searchset.size(); i++) {
        min_distance[i] = INT_MAX;
        nearest_neighbor[i] = -1;
        time[i] = 0;
    }

    /* for every query */
//    cout <<  ANN.size() << ANN[0].size() << ANN[0][0].size() << ANN[0][0][0].size() << endl;
    int calculations = 0;
    for (int q = 0; q < searchset.size(); q++) {
        auto start = chrono::high_resolution_clock::now();
        //find for every query its neighbor vertices
        for (int n = 0; n < ANN[q].size(); n++) {
            calculations = 0;
            for (int j = 0; j < ANN[q][n].size() && calculations < M; j++) {
                distance = dist(&ANN[q][n][j], &searchset[q], dataset[0].size(), Metric);
                if (distance < min_distance[q]) {
                    min_distance[q] = distance;
                    nearest_neighbor[q] = ANN[q][n][j][0] + 1;
                }
                calculations++;
            }
        }
        curr_fraction = (double) min_distance[q] / TrueDistances[q];
        if (curr_fraction > max_af) max_af = curr_fraction;
        average_af += curr_fraction;
        auto finish = chrono::high_resolution_clock::now();
        auto elapsed = finish - start;
        double time_elapsed = chrono::duration<double>(elapsed).count();
        average_time += time_elapsed;
        time[q] = time_elapsed;
    }
    average_af = average_af / searchset.size();
    average_time = average_time / searchset.size();
    cout << "Variables used: | k = " << k << " | dim = " << dim << " | M = " << M << " | probes = " << probes << endl;
    cout << "MAX Approximation Fraction (LSH Distance / True Distance) = " << max_af << endl;
    cout << "Average Approximation Fraction (LSH Distance / True Distance) = " << average_af << endl;
    cout << "Average Time of LSH Distance Computation = " << setprecision(9) << showpoint << fixed << average_time << endl;

    /* print results */
    /* open file to write results */
    ofstream neighbors_file;
    neighbors_file.open ("./output/nneighbors_bhc.txt");
    for (int i = 0; i < searchset.size(); i++) {
        neighbors_file << "Query: " << i + 1 << endl;
        neighbors_file << "Nearest Neighbor: " << nearest_neighbor[i]<< endl;
        neighbors_file << "distanceLSH: " << min_distance[i] << endl;
        neighbors_file << "distanceTrue: " << TrueDistances[i] << endl;
        neighbors_file << "tLSH: " << setprecision(9) << showpoint << fixed << time[i] << endl;
        neighbors_file << "tTrue: " << setprecision(9) << showpoint << fixed << TrueTimes[i] << endl << endl;
    }
    neighbors_file.close();

    return 0;
}
