#include <iostream>
#include <algorithm>
#include <cstring>

#include "def.h"
#include "util.h"
#include "amips.h"

using namespace ip;

// -----------------------------------------------------------------------------
void usage()                        // display the usage of this package
{
    printf("\n"
        "-------------------------------------------------------------------\n"
        " Parameters for k-Maximum Inner Product Search (kMIPS)             \n"
        "-------------------------------------------------------------------\n"
        "    -alg  {integer}  options of algorithms\n"
        "    -n    {integer}  cardinality of the dataset\n"
        "    -qn   {integer}  number of queries\n"
        "    -d    {integer}  dimensionality of the dataset\n"
        "    -K    {integer}  #hash tables for SRP-LSH\n"
        "    -b    {real}     interval ratio for H2-ALSH & SA-ALSH\n"
        "    -ds   {string}   address of data  set\n"
        "    -qs   {string}   address of query set\n"
        "    -ts   {string}   address of truth set\n"
        "    -of   {string}   output folder\n"
        "\n"
        "-------------------------------------------------------------------\n"
        " Primary Options of Algorithms                                     \n"
        "-------------------------------------------------------------------\n"
        "    0  - Ground Truth\n"
        "         Parameters: -alg 0 -n -qn -d -ds -qs -ts\n"
        "\n"
        "    1  - Linear Scan\n"
        "         Parameters: -alg 1 -n -qn -d -ds -qs -ts -of\n"
        "\n"
        "    2  - H2-ALSH\n"
        "         Parameters: -alg 2 -n -qn -d -b -ds -qs -ts -of\n"
        "\n"
        "    3  - SA-ALSH\n"
        "         Parameters: -alg 3 -n -qn -d -K -b -ds -qs -ts -of\n"
        "\n"
        "-------------------------------------------------------------------\n"
        " Author: Qiang Huang (huangq@comp.nus.edu.sg)                      \n"
        "-------------------------------------------------------------------\n"
        "\n\n\n");
}

// -----------------------------------------------------------------------------
int main(int nargs, char **args)
{
    srand(6);                       // setup random seed
    
    int   cnt  = 1;                 // parameter counter
    int   alg  = -1;                // which algorithm?
    int   n    = -1;                // data  cardinality
    int   qn   = -1;                // query cardinality
    int   d    = -1;                // dimensionality
    int   K    = -1;                // # hash tables for SRP-LSH
    float b    = -1.0f;             // interval ratio for H2-ALSH & SA-ALSH
    char  data_addr[200];           // address of data set
    char  query_addr[200];          // address of query set
    char  truth_addr[200];          // address of truth set
    char  out_folder[200];          // output folder
    
    printf("-------------------------------------------------------------\n");
    while (cnt < nargs) {
        if (strcmp(args[cnt], "-alg") == 0) {
            alg = atoi(args[++cnt]); assert(alg >= 0);
            printf("alg = %d\n", alg);
        }
        else if (strcmp(args[cnt], "-n") == 0) {
            n = atoi(args[++cnt]); assert(n > 0);
            printf("n   = %d\n", n);
        }
        else if (strcmp(args[cnt], "-qn") == 0) {
            qn = atoi(args[++cnt]); assert(qn > 0);
            printf("qn  = %d\n", qn);
        }
        else if (strcmp(args[cnt], "-d") == 0) {
            d = atoi(args[++cnt]); assert(d > 0);
            printf("d   = %d\n", d);
        }
        else if (strcmp(args[cnt], "-K") == 0) {
            K = atoi(args[++cnt]); assert(K > 0);
            printf("K   = %d\n", K);
        }
        else if (strcmp(args[cnt], "-b") == 0) {
            b = atof(args[++cnt]); assert(b > 0.0f && b < 1.0f);
            printf("b   = %g\n", b);
        }
        else if (strcmp(args[cnt], "-ds") == 0) {
            strncpy(data_addr, args[++cnt], sizeof(data_addr));
            printf("ds  = %s\n", data_addr);
        }
        else if (strcmp(args[cnt], "-qs") == 0) {
            strncpy(query_addr, args[++cnt], sizeof(query_addr));
            printf("qs  = %s\n", query_addr);
        }
        else if (strcmp(args[cnt], "-ts") == 0) {
            strncpy(truth_addr, args[++cnt], sizeof(truth_addr));
            printf("ts  = %s\n", truth_addr);
        }
        else if (strcmp(args[cnt], "-of") == 0) {
            strncpy(out_folder, args[++cnt], sizeof(out_folder));
            create_dir(out_folder);
            printf("of  = %s\n", out_folder);
        }
        else {
            usage(); exit(1);
        }
        ++cnt;
    }
    printf("-------------------------------------------------------------\n\n");

    // -------------------------------------------------------------------------
    //  read data set, query set, and ground truth file
    // -------------------------------------------------------------------------
    gettimeofday(&g_start_time, nullptr);
    float  *data_set  = new float[(u64)n*d];
    float  *query_set = new float[(u64)qn*d];
    Result *truth_set = nullptr;

    if (read_bin_data(n, d, data_addr,  data_set))  exit(1);
    if (read_bin_data(n, d, query_addr, query_set)) exit(1);
    if (alg > 0) {
        truth_set = new Result[qn*K_MAX];
        if (read_ground_truth(qn, K_MAX, truth_addr, truth_set)) exit(1);
    }
    gettimeofday(&g_end_time, nullptr);
    double input_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    printf("Read items, users, & queries: %g Seconds\n\n", input_time);

    // -------------------------------------------------------------------------
    //  methods
    // -------------------------------------------------------------------------
    switch (alg) {
    case 0:
        ground_truth(n, qn, d, truth_addr, (const float*) data_set,
            (const float*) query_set);
        break;
    case 1:
        linear_scan(n, qn, d, "linear", out_folder, (const float*) data_set, 
            (const float*) query_set, (const Result*) truth_set);
        break;
    case 2:
        h2_alsh(n, qn, d, b, "h2_alsh", out_folder, (const float*) data_set, 
            (const float*) query_set, (const Result*) truth_set);
        break;
    case 3:
        sa_alsh(n, qn, d, K, b, "sa_alsh", out_folder, (const float*) data_set, 
            (const float*) query_set, (const Result*) truth_set);
        break;
    default:
        printf("Parameters error!\n");
        usage();
        break;
    }
    
    // -------------------------------------------------------------------------
    //  release space
    // -------------------------------------------------------------------------
    if (!data_set)  { delete[] data_set;  data_set  = nullptr; }
    if (!query_set) { delete[] query_set; query_set = nullptr; }
    if (!truth_set) { delete[] truth_set; truth_set = nullptr; }
    
    return 0;
}
