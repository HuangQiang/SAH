#pragma once

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

#include "def.h"
#include "util.h"
#include "pri_queue.h"
#include "block.h"

namespace ip {

// -----------------------------------------------------------------------------
//  H2_LOWB: a data structure adapted for performing reverse k-mips
//  
//  Pre-processing Phase:
//  1. compute l2-norms & sort item_set in descending order of l2-norms
//  2. compute l2-norms for user_set
//  3. determine k_max approximate mips results as lower bounds for user_set
//  4. build blocks for the rest item_set (with h2-trans) for batch pruning
//  
//  Online Query Phase:
//  1. for each user, check item_set with blocks for batch pruning
//  2. for each block in item_set, use qalsh (with h2-trans) for speedup
// -----------------------------------------------------------------------------
class H2_LOWB {
public:
    H2_LOWB(                        // constructor
        int   n,                        // item cardinality
        int   m,                        // user cardinality
        int   d,                        // dimensionality
        int   k_max,                    // max k value
        float b,                        // interval ratio for blocking items
        const float *item_set,          // item set
        const float *user_set);         // user set
    
    // -------------------------------------------------------------------------
    ~H2_LOWB();                     // destructor
    
    // -------------------------------------------------------------------------
    void display();                 // display parameters
    
    // -------------------------------------------------------------------------
    void reverse_kmips(             // reverse k-mips
        int   k,                        // top k value
        const float *query,             // query vector
        std::vector<int> &result);      // reverse k-mips result (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {    // get memory usage
        u64 ret = 0;
        ret += sizeof(*this);
        ret += (sizeof(int)+sizeof(float))*n_; // item_index_ & item_norms_
        ret += sizeof(float)*m_;        // user_norms_
        ret += sizeof(float)*m_*k_max_; // lower_bounds_
        for (auto hash : hashs_) {      // hashs_
            ret += hash->get_estimated_memory();
        }
        return ret;
    }
    
protected:
    int   n_;                       // item cardinality
    int   m_;                       // user cardinality
    int   d_;                       // dimensionality
    int   k_max_;                   // max k value
    float b_;                       // interval ratio for blocking items
    
    float *item_set_;               // sorted item vectors
    float *item_norms_;             // sorted item l2-norms
    int   *item_index_;             // sorted item index
    std::vector<Item_Block*> hashs_;// lsh index for item blocks
    
    const float *user_set_;         // user vectors
    float *user_norms_;             // user l2-norms
    float *lower_bounds_;           // lower bounds for user vectors
    
    // -------------------------------------------------------------------------
    void compute_norm_and_sort(     // compute norm and sort data (descending)
        int   n,                        // input set cardinality
        const float *input_set,         // input set
        int   *data_index,              // index of sorted data (return)
        float *data_norm,               // l2-norm of sorted data (return)
        float *data_set);               // sorted data (return)
    
    // -------------------------------------------------------------------------
    void lower_bounds_computation(  // compute lower bounds for user_set
        int n0);                        // the first n0 elements in item_set
    
    // -------------------------------------------------------------------------
    void update_lower_bound(        // update lower bound
        int   k,                        // top-k value
        MaxK_Array *arr,                // top-k array
        float *lower_bound);            // lower bound (return)
    
    // -------------------------------------------------------------------------
    void blocking_item_set(         // split the rest item_set into blocks
        int   n,                        // item cardinality
        const float *item_norms,        // item l2-norms
        const float *item_set);         // item set
    
    // -------------------------------------------------------------------------
    void add_block_by_items(        // add one block by items
        int   n,                        // number of items
        float M,                        // max l2-norm of items
        const float *norms,             // l2-norm of items
        const float *items);            // items 
    
    // -------------------------------------------------------------------------
    int kmips(                      // k-mips
        int   k,                        // top-k value
        float uq_ip,                    // inner product of user and query
        float user_norm,                // l2-norm of input user
        const float *user,              // input user
        MaxK_Array  *arr);              // top-k mips array (return)
};

} // end namespace ip
