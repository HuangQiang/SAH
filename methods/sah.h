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
#include "cone_tree.h"

namespace ip {

// -----------------------------------------------------------------------------
//  SAH: a data structure designed for performing reverse k-mips
//  
//  Pre-processing Phase:
//  1. compute l2-norms & sort item_set in descending order of l2-norms
//  2. build blocks (with cone-tree) for user_set for batch pruning
//  3. build blocks for the rest item_set (with sa-trans) for batch pruning
//  
//  Online Query Phase:
//  1. check user_set with blocks (with cone-tree) for batch pruning
//  2. for each user, check item_set with blocks for batch pruning
//  3. for each block in item_set, use srp-lsh (with sa-trans) for speedup
// -----------------------------------------------------------------------------
class SAH {
public:
    SAH(                            // constructor
        int   n,                        // item cardinality
        int   m,                        // user cardinality
        int   d,                        // dimensionality
        int   k_max,                    // max k value
        int   K,                        // # hash tables for SRP-LSH
        int   leaf,                     // leaf size of cone-tree
        float b,                        // interval ratio for blocking items
        const float *item_set,          // item set
        const float *user_set);         // user set
    
    // -------------------------------------------------------------------------
    ~SAH();                         // destructor
    
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
        for (auto hash : hashs_) {  // hashs_
            ret += hash->get_estimated_memory();
        }
        ret += tree_->get_estimated_memory();
        
        return ret;
    }
    
protected:
    int   n_;                       // item cardinality
    int   m_;                       // user cardinality
    int   d_;                       // dimensionality
    int   k_max_;                   // max k value
    int   K_;                       // # hash tables for SRP-LSH
    int   leaf_;                    // leaf size of cone-tree
    float b_;                       // interval ratio for blocking items
    
    float *item_set_;               // sorted item_set
    float *item_norms_;             // sorted item l2-norms
    int   *item_index_;             // sorted item index
    std::vector<Item_Block*> hashs_;// lsh index for item blocks
    
    Cone_Tree *tree_;               // cone-tree
    std::vector<Cone_Node*> blocks_;// user blocks
    
    // -------------------------------------------------------------------------
    void compute_norm_and_sort(     // compute norm and sort data (descending)
        const float *item_set);         // item_set
    
    // -------------------------------------------------------------------------
    void blocking_user_set(         // build blocks (with cone-tree) for user_set
        int   n0,                       // the first n0 elements in item_set
        const float *user_set);         // user_set
        
    // -------------------------------------------------------------------------
    void lower_bounds_computation(  // compute lower bounds for users
        int   m,                        // number of users
        int   n0,                       // the first n0 elements in item_set
        const float *user_set,          // users
        float *lower_bounds);           // lower bounds (return)
    
    // -------------------------------------------------------------------------
    void update_lower_bound(        // update lower bound
        int   k,                        // top-k value
        MaxK_Array *arr,                // top-k array
        float *lower_bound);            // lower bound (return)
        
    // -------------------------------------------------------------------------
    void node_lower_bounds_computation(// compute lower bound for a node
        int   m,                        // number of users
        const float *lower_bounds,      // lower bounds
        float *node_lower_bounds);      // node lower bounds (return)
    
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
        const float *user,              // input user
        MaxK_Array  *arr);              // top-k mips array (return)
};

} // end namespace ip
