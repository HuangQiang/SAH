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
extern u64    g_cand_cnt;           // global param: # ip computation counter
extern double g_run_time;           // global param: running time (ms)
extern double g_recall;             // global param: recall (%)
extern double g_ratio;              // global param: overall ratio

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
int read_ground_truth(              // read ground truth results from disk
    int    qn,                          // number of query points
    int    k_max,                       // k max value
    const  char *truth_addr,            // address of truth set
    Result *truth_set);                 // ground truth results (return)

// -----------------------------------------------------------------------------
int write_ground_truth(             // write ground truth results to disk
    int   qn,                           // number of query points
    int   k_max,                        // k max value
    const char *truth_addr,             // address of truth set
    const Result *truth_set);           // ground truth results
    
// -----------------------------------------------------------------------------
void write_index_info(              // display & write index overhead info
    FILE *fp);                          // file pointer (return)

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    const char *method_name,            // method name
    FILE  *fp);                         // file pointer (return)

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    float b,                            // interval ratio
    const char *method_name,            // method name
    FILE  *fp);                         // file pointer (return)

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    int   K,                            // # hash tables for SRP-LSH
    float b,                            // interval ratio
    const char *method_name,            // method name
    FILE  *fp);                         // file pointer (return)

// -----------------------------------------------------------------------------
void init_global_metric();          // init the global metric

// -----------------------------------------------------------------------------
void update_global_metric(          // init the global metric
    int   k,                            // top-k value
    const Result *truth,                // ground truth result
    MaxK_List *list);                   // top-k results by a method

// -----------------------------------------------------------------------------
void calc_and_write_global_metric(  // init the global metric
    int  k,                             // top-k value
    int  qn,                            // number of queries
    FILE *fp);                          // file pointer

// -----------------------------------------------------------------------------
void foot(                          // close file
    FILE *fp);                          // file pointer (return)

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
    int   n,                            // number of data points
    int   d,                            // dimensionality
    const float *data,                  // data points
    const float *centroid,              // centroid
    float *shift_data,                  // shifted data points (return)
    float *shift_norms);                // shifted l2-norm sqrs (return)

// -----------------------------------------------------------------------------
void kmips(                         // k-MIPS by linear scan
    int   n,                            // number of data points
    int   d,                            // dimensionality
    const float *data_set,              // data points
    const float *query,                 // query point
    MaxK_List *list);                   // top-k results (return)

} // end namespace ip
