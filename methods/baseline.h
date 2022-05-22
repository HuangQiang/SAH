#pragma once

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>

#include "def.h"
#include "pri_queue.h"
#include "util.h"

namespace ip {

// -----------------------------------------------------------------------------
//  Scan: a data structure for performing reverse k-mips
// 
//  Pre-processing Phase:
//  1. compute l2-norm of all item_set and user_set
//  2. sort item_set in descending order of their l2-norm
//  3. determine and store k_max exact mips results for user_set
//  
//  Online Query Phase:
//  sequential check the user_set
// -----------------------------------------------------------------------------
class Scan {
public:
    int   m_;                       // user cardinality
    int   d_;                       // dimensionality
    int   k_max_;                   // max k value
    
    const float *user_set_;         // user set, O(1)
    float *user_norms_;             // l2-norm of user vectors, O(m)
    float *k_bounds_;               // k bounds for users, O(m*k_max)
    
    // -------------------------------------------------------------------------
    Scan(                           // constructor
        int   n,                        // item cardinality
        int   m,                        // user cardinality
        int   d,                        // dimensionality
        int   k_max,                    // max k value
        bool  parallel,                 // use openmp for parallel computing
        const float *item_set,          // item set
        const float *user_set);         // user set
    
    // -------------------------------------------------------------------------
    ~Scan();                        // destructor
    
    // -------------------------------------------------------------------------
    void display();                 // display parameters
    
    // -------------------------------------------------------------------------
    void reverse_kmips(             // reverse k-mips
        int   k,                        // top k value
        const float *query,             // query vector
        std::vector<int> &result);      // reverse k-mips result (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {    // get estimated memory (bytes)
        uint64_t ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(float)*m_;   // user_norms_
        ret += sizeof(float)*m_*k_max_; // lower_bound_
        
        return ret;
    }
    
    // -------------------------------------------------------------------------
    void compute_norm_and_sort(     // compute l2-norm and sort item_set
        int   n,                        // input set cardinality
        const float *input_set,         // input set
        float *data_norms,              // l2-norm of sorted data (return)
        float *data_set);               // sorted data (return)
    
    // -------------------------------------------------------------------------
    void parallel_k_bounds_computation(// parallel compute k bounds for user_set
        int   n,                        // item cardinality
        const float *item_norms,        // l2-norm of sorted items
        const float *item_set);         // sorted items
    
    // -------------------------------------------------------------------------
    void kmips(                     // k-mips by linear scan with pruning
        int   k,                        // top-k value
        int   n,                        // item cardinality
        float user_norm,                // l2-norm of user
        const float *user,              // user vector
        const float *item_norms,        // l2-norm of sorted items
        const float *item_set,          // sorted items
        MaxK_Array *arr,                // top-k array (return)
        float *k_bounds);               // k bounds (return)
        
    // -------------------------------------------------------------------------
    void update_k_bounds(           // update k bounds
        int   k,                        // top-k value
        MaxK_Array *arr,                // top-k array
        float *k_bounds);               // k bounds (return)
    
    // -------------------------------------------------------------------------
    void k_bounds_computation(      // compute k bounds for user_set
        int   n,                        // item cardinality
        const float *item_norms,        // l2-norm of sorted items
        const float *item_set);         // sorted items
};

} // end namespace ip
