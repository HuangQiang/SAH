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
//  SA_Simpfer: a data structure designed for performing reverse k-mips
//  
//  Pre-processing Phase:
//  1. compute l2-norms & sort item_set in descending order of l2-norms
//  2. compute l2-norms & sort user_set in descending order of l2-norms
//  3. determine k_max approximate mips results as lower bounds for user_set
//  4. build blocks for user_set for batch pruning
//  5. build blocks for the rest item_set (with sa-trans) for batch pruning
//  
//  Online Query Phase:
//  1. check user_set with blocks for batch pruning
//  2. for each user, check item_set with blocks for batch pruning
//  3. for each block in item_set, use srp-lsh (with sa-trans) for speedup
// -----------------------------------------------------------------------------
class SA_Simpfer {
public:
    SA_Simpfer(                     // constructor
        int   n,                        // item cardinality
        int   m,                        // user cardinality
        int   d,                        // dimensionality
        int   k_max,                    // max k value
        int   K,                        // # hash tables for SRP-LSH
        float b,                        // interval ratio for blocking items
        const float *item_set,          // item set
        const float *user_set);         // user set
    
    // -------------------------------------------------------------------------
    ~SA_Simpfer();                  // destructor
    
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
        ret += (sizeof(int)+sizeof(float))*m_; // user_index_ & user_norms_
        ret += sizeof(float)*m_*k_max_; // lower_bounds_
        for (auto hash : hashs_) {      // hashs_
            ret += hash->get_estimated_memory();
        }
        for (auto block : blocks_) {    // blocks_
            ret += block->get_estimated_memory();
        }
        return ret;
    }
    
protected:
    int   n_;                       // item cardinality
    int   m_;                       // user cardinality
    int   d_;                       // dimensionality
    int   k_max_;                   // max k value
    int   K_;                       // # hash tables for SRP-LSH
    float b_;                       // interval ratio for blocking items
    int   cand_cnt_;                // candidate counter for query
    
    float *item_set_;               // sorted item vectors
    float *item_norms_;             // sorted item l2-norms
    int   *item_index_;             // sorted item index
    std::vector<Item_Block*> hashs_;// lsh index for item blocks
    
    float *user_set_;               // sorted user vectors
    float *user_norms_;             // sorted user l2-norms
    int   *user_index_;             // sorted user index
    float *lower_bounds_;           // lower bounds for sorted user vectors
    
    int   block_size_;              // block size of users
    std::vector<User_Block*> blocks_;// user blocks
    
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
    void blocking_user_set();       // split the user_set into blocks
    
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