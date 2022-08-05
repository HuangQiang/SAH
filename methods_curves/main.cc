#include <iostream>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <cstring>
#include <sys/types.h>
#include "omp.h"

#include "def.h"
#include "util.h"
#include "baseline.h"
#include "cone_tree.h"
#include "armips.h"

using namespace ip;

// -----------------------------------------------------------------------------
void usage()                        // display the usage
{
    printf("\n"
        "-------------------------------------------------------------------\n"
        " Parameters for Reverse k-Maximum Inner Product Search (RkMIPS)    \n"
        "-------------------------------------------------------------------\n"
        " -alg   {integer}  options of algorithms\n"
        " -n     {integer}  item  cardinality\n"
        " -m     {integer}  user  cardinality\n"
        " -qn    {integer}  query cardinality\n"
        " -d     {integer}  dimensionality\n"
        " -K     {integer}  # hash tables for SRP-LSH\n"
        " -l     {integer}  leaf size for Cone-Tree/Ball-Tree\n"
        " -b     {real}     interval ratio for H2-ALSH & SA-ALSH\n"
        " -dn    {string}   data name\n"
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
        " 1  - Exhaustive_Scan\n"
        "      Param: -alg 1 -n -m -qn -d -is -us -qs -ts -of\n"
        "\n"
        " 2  - SA_Simpfer (SA-ALSH + Simpfer)\n"
        "      Param: -alg 2 -n -m -qn -d -K -b -dn -is -us -qs -ts -of\n"
        "\n"
        " 3  - SA_Cone (SA-ALSH + Cone-Tree Blocking)\n"
        "      Param: -alg 3 -n -m -qn -d -K -l -b -dn -is -us -qs -ts -of\n"
        "\n"
        " 4  - H2_ALSH\n"
        "      Param: -alg 4 -n -m -qn -d -b -dn -is -us -qs -ts -of\n"
        "\n"
        " 5  - H2_Simpfer (H2-ALSH + Simpfer)\n"
        "      Param: -alg 5 -n -m -qn -d -b -dn -is -us -qs -ts -of\n"
        "\n"
        " 6  - H2_Cone (H2-ALSH + Cone-Tree Blocking)\n"
        "      Param: -alg 6 -n -m -qn -d -l -b -dn -is -us -qs -ts -of\n"
        "\n"
        " 7  - Linear Scan User Set\n"
        "      Param: -alg 7 -n -m -qn -d -is -us -qs -ts -of\n"
        "\n"
        "-------------------------------------------------------------------\n"
        " Author: Qiang Huang (huangq@comp.nus.edu.sg)                      \n"
        "-------------------------------------------------------------------\n"
        "\n\n\n");
}

// -----------------------------------------------------------------------------
void unit_test(                     // unit test Scan and Cone_Tree
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   d,                            // dimensionality
    const float *item_set,              // set of item vectors
    const float *user_set)              // set of user vectors
{
    int k_max = 20;
    int num_q = 10;
    
    omp_set_num_threads(4);
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        printf("Hello (%d) ",    id);
        printf("World (%d)\n\n", id );
    }
    
    Scan *scan1 = new Scan(n, m, d, k_max, true, item_set, user_set);
    for (int i = 0; i < num_q; ++i) {
        float *k_bound = scan1->k_bounds_ + (u64) i*k_max;
        
        for (int j = 0; j < k_max; ++j) printf("%g ", k_bound[j]);
        printf("\n");
    }
    printf("\n");
    
    Scan *scan2 = new Scan(n, m, d, k_max, false, item_set, user_set);
    for (int i = 0; i < num_q; ++i) {
        float *k_bound = scan2->k_bounds_ + (u64) i*k_max;
        
        for (int j = 0; j < k_max; ++j) printf("%g ", k_bound[j]);
        printf("\n");
    }
    printf("\n");
    
    delete scan1;
    delete scan2;
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
    char  data_name[200];           // data set name
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
        else if (strcmp(args[cnt], "-dn") == 0) {
            strncpy(data_name, args[++cnt], sizeof(data_name));
            printf("data_name = %s\n", data_name);
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
    
    // unit_test(n, m, d, item_set, user_set);
    
    // -------------------------------------------------------------------------
    //  methods
    // -------------------------------------------------------------------------
    switch (alg) {
    case 0:
        ground_truth(n, m, qn, d, truth_addr, (const float*) item_set, 
            (const float*) user_set, (const float*) query_set);
        break;
    case 1:
        exhaustive_scan(n, m, qn, d, "exhaustive_scan", truth_addr, 
            out_folder, (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    case 2:
        sa_simpfer(n, m, qn, d, K, b, data_name, "sa_simpfer", truth_addr, 
            out_folder, (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    case 3:
        sa_cone(n, m, qn, d, K, leaf, b, data_name, "sa_cone", truth_addr, 
            out_folder, (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    case 4:
        h2_alsh(n, m, qn, d, b, data_name, "h2_alsh", truth_addr, 
            out_folder, (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    case 5:
        h2_simpfer(n, m, qn, d, b, data_name, "h2_simpfer", truth_addr, 
            out_folder, (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    case 6:
        h2_cone(n, m, qn, d, leaf, b, data_name, "h2_cone", truth_addr, 
            out_folder, (const float*) item_set, (const float*) user_set, 
            (const float*) query_set);
        break;
    case 7:
        linear(m, qn, d, "linear", truth_addr, out_folder, 
            (const float*) user_set, (const float*) query_set);
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
