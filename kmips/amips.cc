#include "amips.h"

namespace ip {

// -----------------------------------------------------------------------------
int ground_truth(                   // find the ground truth results
    int   n,                            // number of data points
    int   qn,                           // number of query points
    int   d,                            // dimensionality
    const char  *truth_addr,            // address of truth set
    const float *data_set,              // data points
    const float *query_set)             // query points
{
    gettimeofday(&g_start_time, NULL);

    // find ground truth results (using linear scan method)
    MaxK_List *list = new MaxK_List(K_MAX);
    Result *truth_set = new Result[qn*K_MAX];
    
    for (int i = 0; i < qn; ++i) {
        // k-mips
        const float *query = query_set + (u64) i*d;
        kmips(n, d, data_set, query, list);
        
        // get the ground truth
        Result *truth = truth_set + i*K_MAX;
        for (int j = 0; j < K_MAX; ++j) {
            truth[j].id_  = list->ith_id(j);
            truth[j].key_ = list->ith_key(j);
        }
    }
    // write ground truth results to disk
    write_ground_truth(qn, K_MAX, truth_addr, truth_set);

    // release space
    delete list;
    delete[] truth_set;

    gettimeofday(&g_end_time, NULL);
    float truth_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0f;
    printf("Ground Truth: %f Seconds\n\n", truth_time);
    
    return 0;
}

// -----------------------------------------------------------------------------
int linear_scan(                    // Linear Scan
    int   n,                            // number of data points
    int   qn,                           // number of query points
    int   d,                            // dimensionality
    const char *method_name,            // method name
    const char *out_folder,             // output folder
    const float *data_set,              // data points
    const float *query_set,             // query points
    const Result *truth_set)            // ground truth results
{
    char fname[200]; sprintf(fname, "%smips_%s.csv", out_folder, method_name);
    FILE *fp = fopen(fname, "a+");
    if (!fp) { printf("Could not create %s\n", fname); return 1; }
    
    // k-maximum inner product search
    write_params(method_name, fp);
    for (int k : Ks) {
        MaxK_List *list = new MaxK_List(k);
        init_global_metric();
        
        for (int i = 0; i < qn; ++i) {
            const float  *query = query_set + (u64) i*d;
            const Result *truth = truth_set + i*K_MAX;
            
            kmips(n, d, data_set, query, list);
            update_global_metric(k, truth, list);
        }
        delete list;
        calc_and_write_global_metric(k, qn, fp);
    }
    foot(fp);
    fclose(fp);

    return 0;
}

// -----------------------------------------------------------------------------
int h2_alsh(                        // H2-ALSH
    int   n,                            // number of data points
    int   qn,                           // number of query points
    int   d,                            // dimensionality
    float b,                            // interval ratio
    const char *method_name,            // method name
    const char *out_folder,             // output folder
    const float *data_set,              // data points
    const float *query_set,             // query points
    const Result *truth_set)            // ground truth results
{
    char fname[200]; sprintf(fname, "%smips_%s.csv", out_folder, method_name);
    FILE *fp = fopen(fname, "a+");
    if (!fp) { printf("Could not create %s\n", fname); return 1; }
    
    // pre-processing
    H2_ALSH *lsh = new H2_ALSH(n, d, b, data_set);
    lsh->display();
    write_index_info(fp);
    
    // k-maximum inner product search
    write_params(b, method_name, fp);
    for (int k : Ks) {
        MaxK_List *list = new MaxK_List(k);
        init_global_metric();
        
        for (int i = 0; i < qn; ++i) {
            const float  *query = query_set + (u64) i*d;
            const Result *truth = truth_set + i*K_MAX;
            
            lsh->kmips(k, query, list);
            update_global_metric(k, truth, list);
        }
        delete list;
        calc_and_write_global_metric(k, qn, fp);
    }
    foot(fp);
    fclose(fp);

    return 0;
}

// -----------------------------------------------------------------------------
int sa_alsh(                        // SA-ALSH
    int   n,                            // number of data points
    int   qn,                           // number of query points
    int   d,                            // dimensionality
    int   K,                            // # hash tables for SRP-LSH
    float b,                            // interval ratio
    const char *method_name,            // method name
    const char *out_folder,             // output folder
    const float *data_set,              // data points
    const float *query_set,             // query points
    const Result *truth_set)            // ground truth results
{
    char fname[200]; sprintf(fname, "%smips_%s.csv", out_folder, method_name);
    FILE *fp = fopen(fname, "a+");
    if (!fp) { printf("Could not create %s\n", fname); return 1; }
    
    // pre-processing
    SA_ALSH *lsh = new SA_ALSH(n, d, K, b, data_set);
    lsh->display();
    write_index_info(fp);
    
    // k-maximum inner product search
    write_params(K, b, method_name, fp);
    for (int k : Ks) {
        MaxK_List *list = new MaxK_List(k);
        init_global_metric();
        
        for (int i = 0; i < qn; ++i) {
            const float  *query = query_set + (u64) i*d;
            const Result *truth = truth_set + i*K_MAX;
            
            lsh->kmips(k, query, list);
            update_global_metric(k, truth, list);
        }
        delete list;
        calc_and_write_global_metric(k, qn, fp);
    }
    foot(fp);
    fclose(fp);

    return 0;
}

} // end namespace ip
