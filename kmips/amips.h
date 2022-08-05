#pragma once

#include <iostream>
#include <algorithm>
#include <sys/time.h>

#include "def.h"
#include "util.h"
#include "pri_queue.h"
#include "h2_alsh.h"
#include "sa_alsh.h"

namespace ip {

// -----------------------------------------------------------------------------
int ground_truth(                   // find the ground truth results
    int   n,                            // number of data points
    int   qn,                           // number of query points
    int   d,                            // dimensionality
    const char  *truth_addr,            // address of truth set
    const float *data_set,              // data points
    const float *query_set);            // query points
    
// -----------------------------------------------------------------------------
int linear_scan(                    // Linear Scan
    int   n,                            // number of data points
    int   qn,                           // number of query points
    int   d,                            // dimensionality
    const char *method_name,            // method name
    const char *out_folder,             // output folder
    const float *data_set,              // data points
    const float *query_set,             // query points
    const Result *truth_set);           // ground truth results

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
    const Result *truth_set);           // ground truth results

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
    const Result *truth_set);           // ground truth results

} // end namespace ip
