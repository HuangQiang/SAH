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
//  H2_ALSH: a data structure adapted for performing reverse k-mips
//  
//  Pre-processing Phase:
//  1. compute l2-norms & sort item_set in descending order of l2-norms
//  2. compute l2-norms for user_set
//  3. build blocks for the rest item_set (with h2-trans) for batch pruning
//  
//  Online Query Phase:
//  for each block in item_set, use qalsh (with h2-trans) for speedup
// -----------------------------------------------------------------------------
class H2_ALSH {
public:
    H2_ALSH(                        // constructor
        int   n,                        // item cardinality
        int   m,                        // user cardinality
        int   d,                        // dimensionality
        float b,                        // interval ratio for blocking items
        const float *item_set,          // item set
        const float *user_set);         // user set
    
    // -------------------------------------------------------------------------
    ~H2_ALSH();                     // destructor
    
    // -------------------------------------------------------------------------
    void display();                 // display parameters
    
    // -------------------------------------------------------------------------
    void reverse_kmips(             // reverse k-mips
        int   k,                        // top k value
        int   cand,                     // # candidates
        const float *query,             // query vector
        std::vector<int> &result);      // reverse k-mips result (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {    // get memory usage
        u64 ret = 0;
        ret += sizeof(*this);
        ret += (sizeof(int)+sizeof(float))*n_; // item_index_ & item_norms_
        ret += sizeof(float)*m_;    // user_norms_
        for (auto hash : hashs_) {  // hashs_
            ret += hash->get_estimated_memory();
        }
        return ret;
    }
    
protected:
    int   n_;                       // item cardinality
    int   m_;                       // user cardinality
    int   d_;                       // dimensionality
    float b_;                       // interval ratio for blocking items
    int   cand_cnt_;                // candidate counter for query
    
    float *item_set_;               // sorted item vectors
    float *item_norms_;             // sorted item l2-norms
    int   *item_index_;             // sorted item index
    std::vector<Item_Block*> hashs_;// lsh index for item blocks
    
    float *user_norms_;             // user l2-norms
    const float *user_set_;         // user vectors
    
    // -------------------------------------------------------------------------
    void compute_norm_and_sort(     // compute norm and sort data (descending)
        int   n,                        // input set cardinality
        const float *input_set,         // input set
        int   *data_index,              // index of sorted data (return)
        float *data_norm,               // l2-norm of sorted data (return)
        float *data_set);               // sorted data (return)
    
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
        int   cand,                     // # candidates
        float uq_ip,                    // inner product of user and query
        float user_norm,                // l2-norm of input user
        const float *user,              // input user
        MaxK_Array  *arr);              // top-k mips array (return)
};

} // end namespace ip
