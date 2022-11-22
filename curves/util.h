#pragma once

#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include <omp.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include "def.h"
#include "pri_queue.h"

namespace ip {

extern timeval g_start_time;        // global param: start time
extern timeval g_end_time;          // global param: end time

extern double g_pre_time;           // global param: pre-processing time (ms)
extern u64    g_memory;             // global param: memory usage (bytes)

extern u64    g_ip_count;           // global param: # ip computation counter
extern int    g_nq_count;           // global param: # non-empty query counter
extern int    g_nq_found;           // global param: # non-empty query found
extern double g_run_time;           // global param: running time (ms)
extern double g_recall;             // global param: recall (%)
extern double g_precision;          // global param: precision (%)
extern double g_f1score;            // global param: f1-score (%)

// -----------------------------------------------------------------------------
//  Input & Output
// -----------------------------------------------------------------------------
void create_dir(                    // create dir if the path exists
    char *path);                        // input path

// -----------------------------------------------------------------------------
int read_bin_data(                  // read binary data from disk
    int   n,                            // number of data
    int   d,                            // dimensionality
    const char *fname,                  // address of data
    float *data);                       // data (return)

// -----------------------------------------------------------------------------
int get_conf(                       // get cand list from configuration file
    const char *data_name,              // name of dataset
    const char *method_name,            // name of method
    std::vector<int> &cand);            // candidates list (return)
    
// -----------------------------------------------------------------------------
int read_ground_truth(              // read ground truth results from disk
    int   k,                            // top-k value
    int   qn,                           // number of query objects
    const char *truth_addr,             // address of truth set
    std::vector<std::vector<int> > &truth); // ground truth results (return)

// -----------------------------------------------------------------------------
int write_ground_truth(             // write ground truth results to disk
    int   k,                            // top-k value
    int   qn,                           // query cardinality
    const char *truth_addr,             // address of truth set
    std::vector<std::vector<int> > &truth); // truth result (allow modify)

// -----------------------------------------------------------------------------
void output_reverse_kmips_results(  // output reverse kmips result for query
    int   i,                            // ith query
    const std::vector<int> &result,     // reverse kmips result
    const char *fname);                 // address of output file

// -----------------------------------------------------------------------------
void write_index_info(              // display & write index overhead info
    FILE *fp);                          // file pointer (return)

// -----------------------------------------------------------------------------
void head(                          // display head with method name
    const char *method_name);           // method name

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    float b,                            // interval ratio for blocking items
    const char *method_name,            // method name
    FILE  *fp);                         // file pointer (return)

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    int   K,                            // # hash tables for SRP-LSH
    float b,                            // interval ratio for blocking items
    const char *method_name,            // method name
    FILE  *fp);                         // file pointer (return)

// -----------------------------------------------------------------------------
void write_params_leaf(             // write parameters
    int   leaf,                         // leaf size of Cone-Tree
    float b,                            // interval ratio for blocking items
    const char *method_name,            // method name
    FILE  *fp);                         // file pointer (return)

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    int   K,                            // # hash tables for SRP-LSH
    int   leaf,                         // leaf size of Cone-Tree
    float b,                            // interval ratio for blocking items
    const char *method_name,            // method name
    FILE  *fp);                         // file pointer (return)

// -----------------------------------------------------------------------------
void foot(                          // close file
    FILE *fp);                          // file pointer (return)

// -----------------------------------------------------------------------------
void init_global_metric();          // init the global metric

// -----------------------------------------------------------------------------
void update_global_metric(          // init the global metric
    const std::vector<int> &truth,      // ground truth result
    std::vector<int> &result);          // result from a method (allow modify)

// -----------------------------------------------------------------------------
void calc_and_write_global_metric(  // init the global metric
    int  top_k,                         // top-k value
    int  cand,                          // # candidates
    int  qn,                            // number of queries
    FILE *fp);                          // file pointer

// -----------------------------------------------------------------------------
//  Generate random variables
// -----------------------------------------------------------------------------
float uniform(                      // r.v. from Uniform(min, max)
    float min,                          // min value
    float max);                         // max value

// -----------------------------------------------------------------------------
float gaussian(                     // r.v. from Gaussian(mean, sigma)
    float mean,                         // mean value
    float sigma);                       // std value

// -----------------------------------------------------------------------------
inline float normal_pdf(            // pdf of Guassian(mean, std)
    float x,                            // variable
    float u,                            // mean
    float sigma)                        // standard error
{
    float ret = exp(-(x - u) * (x - u) / (2.0f * sigma * sigma));
    ret /= sigma * sqrt(2.0f * PI);
    
    return ret;
}

// -----------------------------------------------------------------------------
float normal_cdf(                   // cdf of N(0, 1) in (-inf, x]
    float x,                            // integral border
    float step = 0.001f);               // step increment

// -----------------------------------------------------------------------------
float new_cdf(                      // cdf of N(0, 1) in [-x, x]
    float x,                            // integral border
    float step = 0.001f);               // step increment

// -----------------------------------------------------------------------------
//  Distance and similarity functions
// -----------------------------------------------------------------------------
float calc_l2_sqr(                  // calc l_2 distance square
    int   dim,                          // dimensionality
    const float *p1,                    // 1st point
    const float *p2);                   // 2nd point

// -----------------------------------------------------------------------------
float calc_l2_dist(                 // calc l_2 distance
    int   dim,                          // dimensionality
    const float *p1,                    // 1st point
    const float *p2);                   // 2nd point

// -----------------------------------------------------------------------------
float calc_inner_product(           // calc inner product
    int   dim,                          // dimensionality
    const float *p1,                    // 1st point
    const float *p2);                   // 2nd point

// -----------------------------------------------------------------------------
float calc_cosine_angle(            // calc cosine angle, [-1,1]
    int   dim,                          // dimensionality
    const float *p1,                    // 1st point
    const float *p2);                   // 2nd point

// -----------------------------------------------------------------------------
float calc_angle(                   // calc angle between two points
    int   dim,                          // dimension
    const float *p1,                    // 1st point
    const float *p2);                   // 2nd point

// -----------------------------------------------------------------------------
void calc_centroid(                 // calc centroid
    int   n,                            // number of data points
    int   dim,                          // dimensionality
    const float *data,                  // data points
    float *centroid);                   // centroid (return)

// -----------------------------------------------------------------------------
void calc_centroid(                 // calc centroid
    int   n,                            // size of data index
    int   dim,                          // dimensionality
    const int   *index,                 // data index
    const float *data,                  // data points
    float *centroid);                   // centroid (return)

// -----------------------------------------------------------------------------
float shift_data_and_norms(         // calc shifted data and their l2-norm sqrs
    int   n,                            // number of data vectors
    int   d,                            // dimensionality
    const float *data,                  // data vectors
    const float *centroid,              // centroid
    float *shift_data,                  // shifted data vectors (return)
    float *shift_norms);                // shifted l2-norm sqrs (return)

// -----------------------------------------------------------------------------
void linear_scan(                   // linear scan user_set for k-mips
    int m,                              // number of user vectors
    int k,                              // top-k value
    int d,                              // dimensionality
    const float *query,                 // query vector
    const float *user_set,              // user vectors
    std::vector<int> &result);          // top-k results (return)

} // end namespace ip
