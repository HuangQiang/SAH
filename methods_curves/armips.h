#pragma once

#include <cstdint>
#include "def.h"
#include "pri_queue.h"
#include "util.h"

#include "baseline.h"
#include "h2_alsh.h"
#include "h2_simpfer.h"
#include "h2_cone.h"
#include "sa_simpfer.h"
#include "sa_cone.h"

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
    const float *query_set);            // set of query vectors

// -----------------------------------------------------------------------------
int exhaustive_scan(                // Exhaustive_Scan
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set);            // set of query vectors

// -----------------------------------------------------------------------------
int sa_simpfer(                     // SA_ALSH + Simpfer
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    int   K,                            // # hash tables
    float b,                            // interval ratio for blocking items
    const char  *data_name,             // name of dataset
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set);            // set of query vectors

// -----------------------------------------------------------------------------
int sa_cone(                        // SA_ALSH + Cone-Tree Blocking
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    int   K,                            // # hash tables
    int   leaf,                         // leaf size of Cone-Tree
    float b,                            // interval ratio for blocking items
    const char  *data_name,             // name of dataset
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set);            // set of query vectors

// -----------------------------------------------------------------------------
int h2_alsh(                        // H2_ALSH
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    float b,                            // interval ratio for blocking items
    const char  *data_name,             // name of dataset
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set);            // set of query vectors

// -----------------------------------------------------------------------------
int h2_simpfer(                     // H2_ALSH + Simpfer
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    float b,                            // interval ratio for blocking items
    const char  *data_name,             // name of dataset
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set);            // set of query vectors

// -----------------------------------------------------------------------------
int h2_cone(                        // H2_ALSH + Cone-Tree Blocking
    int   n,                            // item  cardinality
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    int   leaf,                         // leaf size of Cone-Tree
    float b,                            // interval ratio for blocking items
    const char  *data_name,             // name of dataset
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *item_set,              // set of item  vectors
    const float *user_set,              // set of user  vectors
    const float *query_set);            // set of query vectors
    
// -----------------------------------------------------------------------------
int linear(                         // Linear Scan k-MIPS
    int   m,                            // user  cardinality
    int   qn,                           // query cardinality
    int   d,                            // dimensionality
    const char  *method_name,           // method name
    const char  *truth_addr,            // address of truth set
    const char  *out_folder,            // output folder
    const float *user_set,              // set of user  vectors
    const float *query_set);            // set of query vectors

} // end namespace ip
