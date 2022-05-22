#include <iostream>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <cstring>
#include <sys/types.h>
#include "omp.h"

#include "def.h"
#include "util.h"
#include "armips.h"

using namespace ip;

// -----------------------------------------------------------------------------
void usage()                        // display the usage
{
    printf("\n"
        "-------------------------------------------------------------------\n"
        " The Parameters for Reverse k-Maximum Inner Product Search         \n"
        "-------------------------------------------------------------------\n"
        " -alg   {integer}  options of algorithms\n"
        " -n     {integer}  item  cardinality\n"
        " -m     {integer}  user  cardinality\n"
        " -qn    {integer}  query cardinality\n"
        " -d     {integer}  dimensionality\n"
        " -K     {integer}  # hash tables for SRP-LSH\n"
        " -l     {integer}  leaf size for Cone-Tree\n"
        " -b     {real}     interval ratio for H2-ALSH & SA-ALSH\n"
        " -is    {string}   address of item  set\n"
        " -us    {string}   address of user  set\n"
        " -qs    {string}   address of query set\n"
        " -ts    {string}   address of truth set\n"
        " -of    {string}   output folder\n"
        "\n"
        "-------------------------------------------------------------------\n"
        " Primary Options of Algorithms                                     \n"
        "-------------------------------------------------------------------\n"
        " 0  - Ground Truth & Histogram & Heatmap\n"
        "      Param: -alg 0 -n -m -qn -d -is -us -qs -ts -of\n"
        "\n"
        " 1  - H2_ALSH\n"
        "      Param: -alg 1 -n -m -qn -d -b -is -us -qs -ts -of\n"
        "\n"
        " 2  - H2_Simpfer\n"
        "      Param: -alg 2 -n -m -qn -d -b -is -us -qs -ts -of\n"
        "\n"
        " 3  - SA_Simpfer\n"
        "      Param: -alg 3 -n -m -qn -d -K -b -is -us -qs -ts -of\n"
        "\n"
        " 4  - SAH\n"
        "      Param: -alg 4 -n -m -qn -d -K -l -b -is -us -qs -ts -of\n"
        "\n"
        "-------------------------------------------------------------------\n"
        " Author: Qiang Huang (huangq@comp.nus.edu.sg)                      \n"
        "-------------------------------------------------------------------\n"
        "\n\n\n");
}

// -----------------------------------------------------------------------------
int main(int nargs, char **args)
{
    srand(RANDOM_SEED);             // setup random seed

    int   cnt  = 1;                 // parameter counter
    int   alg  = -1;                // which algorithm?
    int   n    = -1;                // item  cardinality
    int   m    = -1;                // user  cardinality
    int   qn   = -1;                // query cardinality
    int   d    = -1;                // dimensionality
    int   K    = -1;                // # hash tables for SRP-LSH
    int   leaf = -1;                // leaf size for Cone-Tree
    float b    = -1.0f;             // interval ratio for H2-ALSH & SA-ALSH
    char  items_addr[200];          // address of item  set
    char  users_addr[200];          // address of user  set
    char  query_addr[200];          // address of query set
    char  truth_addr[200];          // address of truth set
    char  out_folder[200];          // output folder

    printf("-------------------------------------------------------------\n");
    while (cnt < nargs) {
        if (strcmp(args[cnt], "-alg") == 0) {
            alg = atoi(args[++cnt]); assert(alg >= 0);
            printf("alg  = %d\n", alg);
        }
        else if (strcmp(args[cnt], "-n") == 0) {
            n = atoi(args[++cnt]); assert(n > 0);
            printf("n    = %d\n", n);
        }
        else if (strcmp(args[cnt], "-m") == 0) {
            m = atoi(args[++cnt]); assert(m > 0);
            printf("m    = %d\n", m);
        }
        else if (strcmp(args[cnt], "-qn") == 0) {
            qn = atoi(args[++cnt]); assert(qn > 0);
            printf("qn   = %d\n", qn);
        }
        else if (strcmp(args[cnt], "-d") == 0) {
            d = atoi(args[++cnt]); assert(d > 0);
            printf("d    = %d\n", d);
        }
        else if (strcmp(args[cnt], "-K") == 0) {
            K = atoi(args[++cnt]); assert(K > 0);
            printf("K    = %d\n", K);
        }
        else if (strcmp(args[cnt], "-l") == 0) {
            leaf = atoi(args[++cnt]); assert(leaf > 0);
            printf("leaf = %d\n", leaf);
        }
        else if (strcmp(args[cnt], "-b") == 0) {
            b = atof(args[++cnt]); assert(b > 0.0f && b < 1.0f);
            printf("b    = %g\n", b);
        }
        else if (strcmp(args[cnt], "-is") == 0) {
            strncpy(items_addr, args[++cnt], sizeof(items_addr));
            printf("is   = %s\n", items_addr);
        }
        else if (strcmp(args[cnt], "-us") == 0) {
            strncpy(users_addr, args[++cnt], sizeof(users_addr));
            printf("us   = %s\n", users_addr);
        }
        else if (strcmp(args[cnt], "-qs") == 0) {
            strncpy(query_addr, args[++cnt], sizeof(query_addr));
            printf("qs   = %s\n", query_addr);
        }
        else if (strcmp(args[cnt], "-ts") == 0) {
            strncpy(truth_addr, args[++cnt], sizeof(truth_addr));
            create_dir(truth_addr);
            printf("ts   = %s\n", truth_addr);
        }
        else if (strcmp(args[cnt], "-of") == 0) {
            strncpy(out_folder, args[++cnt], sizeof(out_folder));
            create_dir(out_folder);
            printf("of   = %s\n", out_folder);
        }
        else {
            usage(); exit(1);
        }
        ++cnt;
    }
    printf("-------------------------------------------------------------\n\n");
    
    // -------------------------------------------------------------------------
    //  read item set, user set, and query set
    // -------------------------------------------------------------------------
    gettimeofday(&g_start_time, nullptr);
    float *item_set  = new float[(u64) n*d];
    float *user_set  = new float[(u64) m*d];
    float *query_set = new float[(u64) qn*d];
    
    if (read_bin_data(n,  d, items_addr, item_set))  exit(1);
    if (read_bin_data(m,  d, users_addr, user_set))  exit(1);
    if (read_bin_data(qn, d, query_addr, query_set)) exit(1);
    
    gettimeofday(&g_end_time, nullptr);
    double input_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    printf("Read items, users, & queries: %g Seconds\n\n", input_time);
    
    // -------------------------------------------------------------------------
    //  methods
    // -------------------------------------------------------------------------
    switch (alg) {
    case 0:
        ground_truth(n, m, qn, d, truth_addr, (const float*) item_set, 
            (const float*) user_set, (const float*) query_set);
        break;
    case 1:
        h2_alsh(n, m, qn, d, b, "h2_alsh", truth_addr, out_folder, 
            (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    case 2:
        h2_simpfer(n, m, qn, d, b, "h2_simpfer", truth_addr, out_folder, 
            (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    case 3:
        sa_simpfer(n, m, qn, d, K, b, "sa_simpfer", truth_addr, out_folder, 
            (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    case 4:
        sah(n, m, qn, d, K, leaf, b, "sah", truth_addr, out_folder, 
            (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    default:
        printf("Parameters error!\n"); usage();
        break;
    }
    
    // -------------------------------------------------------------------------
    //  release space
    // -------------------------------------------------------------------------
    if (!item_set)  { delete[] item_set;  item_set  = nullptr; }
    if (!user_set)  { delete[] user_set;  user_set  = nullptr; }
    if (!query_set) { delete[] query_set; query_set = nullptr; }
    
    return 0;
}