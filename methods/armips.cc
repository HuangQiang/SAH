#pragma once

#include <cstdint>
#include "armips.h"

namespace ip {

// -----------------------------------------------------------------------------
int ground_truth(                   // generate ground truth results for query
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    const char  *truth_addr,            // address of truth set
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set)             // set of query vectors
{
    // pre-processing
    Scan *scan = new Scan(n, m, d, K_MAX, true, item_set, user_set);
    scan->display();
    
    // find ground truth results
    std::vector<std::vector<int> > truth; // ground truth
    std::vector<int> result;              // results by this method
    
    for (int k : Ks) {
        init_global_metric();
        
        for (int i = 0; i < qn; ++i) {
            const float *query = query_set + (u64) i*d;
            scan->reverse_kmips(k, query, result);
            truth.push_back(result);
        }
        if (write_ground_truth(k, qn, truth_addr, truth)) return 1;
        
        std::vector<std::vector<int> >().swap(truth);
    }
    delete scan;
    return 0;
}

// -----------------------------------------------------------------------------
int h2_alsh(                        // H2_ALSH
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    float b,                            // interval ratio for blocking items
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set)             // set of query vectors
{
    char fname[200]; sprintf(fname, "%s%s.csv", out_folder, method_name);
    FILE *fp = fopen(fname, "a+");
    if (!fp) { printf("Could not create %s\n", fname); return 1; }
    
    // pre-processing
    H2_ALSH *lsh = new H2_ALSH(n, m, d, b, item_set, user_set);
    lsh->display();
    write_index_info(fp);
    
    // online query for reverse k-maximum inner product search
    std::vector<std::vector<int> > truth; // ground truth
    std::vector<int> result;              // results by this method
    
    write_params(b, method_name, fp);
    for (int k : Ks) {
        // char k_fname[200]; 
        // sprintf(k_fname, "%s%s_k=%d.csv", out_folder, method_name, k);
        
        read_ground_truth(k, qn, truth_addr, truth);
        init_global_metric();
        
        for (int i = 0; i < qn; ++i) {
            const float *query = query_set + (u64) i*d;
            lsh->reverse_kmips(k, query, result);
            
            update_global_metric(truth[i], result);
            // output_reverse_kmips_results(i, result, k_fname);
        }
        calc_and_write_global_metric(k, qn, fp);
        std::vector<std::vector<int> >().swap(truth);
        std::vector<int>().swap(result);
    }
    foot(fp);
    fclose(fp);
    
    delete lsh;
    return 0;
}

// -----------------------------------------------------------------------------
int h2_simpfer(                     // H2_ALSH with Simpfer optimizations
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    float b,                            // interval ratio for blocking items
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set)             // set of query vectors
{
    char fname[200]; sprintf(fname, "%s%s.csv", out_folder, method_name);
    FILE *fp = fopen(fname, "a+");
    if (!fp) { printf("Could not create %s\n", fname); return 1; }
    
    // pre-processing
    H2_Simpfer *lsh = new H2_Simpfer(n, m, d, K_MAX, b, item_set, user_set);
    lsh->display();
    write_index_info(fp);
    
    // online query for reverse k-maximum inner product search
    std::vector<std::vector<int> > truth; // ground truth
    std::vector<int> result;              // results by this method
    
    write_params(b, method_name, fp);
    for (int k : Ks) {
        // char k_fname[200]; 
        // sprintf(k_fname, "%s%s_k=%d.csv", out_folder, method_name, k);
        
        read_ground_truth(k, qn, truth_addr, truth);
        init_global_metric();
        
        for (int i = 0; i < qn; ++i) {
            const float *query = query_set + (u64) i*d;
            lsh->reverse_kmips(k, query, result);
            
            update_global_metric(truth[i], result);
            // output_reverse_kmips_results(i, result, k_fname);
        }
        calc_and_write_global_metric(k, qn, fp);
        std::vector<std::vector<int> >().swap(truth);
        std::vector<int>().swap(result);
    }
    foot(fp);
    fclose(fp);
    
    delete lsh;
    return 0;
}

// -----------------------------------------------------------------------------
int sa_simpfer(                     // SA_ALSH with Simpfer optimizations
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    int   K,                            // # hash tables
    float b,                            // interval ratio for blocking items
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set)             // set of query vectors
{
    char fname[200]; sprintf(fname, "%s%s.csv", out_folder, method_name);
    FILE *fp = fopen(fname, "a+");
    if (!fp) { printf("Could not create %s\n", fname); return 1; }
    
    // pre-processing
    SA_Simpfer *lsh = new SA_Simpfer(n, m, d, K_MAX, K, b, item_set, user_set);
    lsh->display();
    write_index_info(fp);
    
    // online query for reverse k-maximum inner product search
    std::vector<std::vector<int> > truth; // ground truth
    std::vector<int> result;              // results by this method
    
    write_params(K, b, method_name, fp);
    for (int k : Ks) {
        // char k_fname[200]; 
        // sprintf(k_fname, "%s%s_k=%d.csv", out_folder, method_name, k);
        
        read_ground_truth(k, qn, truth_addr, truth);
        init_global_metric();
        
        for (int i = 0; i < qn; ++i) {
            const float *query = query_set + (u64) i*d;
            lsh->reverse_kmips(k, query, result);
            
            update_global_metric(truth[i], result);
            // output_reverse_kmips_results(i, result, k_fname);
        }
        calc_and_write_global_metric(k, qn, fp);
        std::vector<std::vector<int> >().swap(truth);
        std::vector<int>().swap(result);
    }
    foot(fp);
    fclose(fp);
    
    delete lsh;
    return 0;
}

// -----------------------------------------------------------------------------
int sah(                            // SAH
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    int   K,                            // # hash tables
    int   leaf,                         // leaf size of Cone-Tree
    float b,                            // interval ratio for blocking items
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set)             // set of query vectors
{
    char fname[200]; sprintf(fname, "%s%s_%d.csv", out_folder, method_name, K);
    FILE *fp = fopen(fname, "a+");
    if (!fp) { printf("Could not create %s\n", fname); return 1; }
    
    // normalized the user set
    float *norm_user_set = new float[m*d];
    for (int i = 0; i < m; ++i) {
        const float *user = user_set + (u64) i*d;
        float norm = sqrt(calc_inner_product(d, user, user));
        
        float *new_user = norm_user_set + (u64)i*d;
        for (int j = 0; j < d; ++j) new_user[j] = user[j] / norm;
    }
    
    // pre-processing
    SAH *lsh = new SAH(n, m, d, K_MAX, K, leaf, b, item_set, norm_user_set);
    lsh->display();
    write_index_info(fp);
    
    // online query for reverse k-maximum inner product search
    std::vector<std::vector<int> > truth; // ground truth
    std::vector<int> result;              // results by this method
    
    write_params(K, leaf, b, method_name, fp);
    for (int k : Ks) {
        // char k_fname[200]; 
        // sprintf(k_fname, "%s%s_k=%d.csv", out_folder, method_name, k);
        
        read_ground_truth(k, qn, truth_addr, truth);
        init_global_metric();
        
        for (int i = 0; i < qn; ++i) {
            const float *query = query_set + (u64) i*d;
            lsh->reverse_kmips(k, query, result);
            
            update_global_metric(truth[i], result);
            // output_reverse_kmips_results(i, result, k_fname);
        }
        calc_and_write_global_metric(k, qn, fp);
        std::vector<std::vector<int> >().swap(truth);
        std::vector<int>().swap(result);
    }
    foot(fp);
    fclose(fp);
    
    delete lsh;
    delete[] norm_user_set;
    return 0;
}

} // end namespace ip