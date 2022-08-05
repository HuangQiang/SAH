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
//  SA_ALSH: a data structure designed for performing k-mips
// -----------------------------------------------------------------------------
class SA_ALSH {
public:
    SA_ALSH(                        // constructor
        int   n,                        // data cardinality
        int   d,                        // dimensionality
        int   K,                        // # hash tables for SRP-LSH
        float b,                        // interval ratio for blocking data
        const float *data_set);         // data set
    
    // -------------------------------------------------------------------------
    ~SA_ALSH();                     // destructor
    
    // -------------------------------------------------------------------------
    void display();                 // display parameters
    
    // -------------------------------------------------------------------------
    void kmips(                     // k-mips
        int   k,                        // top k value
        const float *query,             // query vector
        MaxK_List *list);               // k-mips result (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {    // get memory usage
        u64 ret = 0;
        ret += sizeof(*this);
        ret += (sizeof(int)+sizeof(float))*n_; // data_index_ & data_norms_
        for (auto hash : hashs_) {  // hashs_
            ret += hash->get_estimated_memory();
        }
        return ret;
    }
    
protected:
    int   n_;                       // data cardinality
    int   d_;                       // dimensionality
    int   K_;                       // # hash tables for SRP-LSH
    float b_;                       // interval ratio for blocking data
    
    int   *data_index_;             // sorted data index, O(n)
    float *data_norms_;             // sorted data l2-norms, O(n)
    float *data_set_;               // sorted data points, O(nd)
    std::vector<Block*> hashs_;     // lsh index for data blocks
    
    // -------------------------------------------------------------------------
    void compute_norm_and_sort(     // compute norm and sort data (descending)
        int   n,                        // input set cardinality
        const float *input_set,         // input set
        int   *data_index,              // index of sorted data (return)
        float *data_norms,              // l2-norm of sorted data (return)
        float *data_set);               // sorted data (return)
    
    // -------------------------------------------------------------------------
    void blocking_data_set(         // split the data_set into blocks
        int   n,                        // data cardinality
        const int   *data_index,        // data index
        const float *data_norms,        // data l2-norms
        const float *data_set);         // data set
    
    // -------------------------------------------------------------------------
    void add_block_by_data_points(  // add one block by data points
        int   n,                        // number of data points
        float M,                        // max l2-norm of data points
        const int   *index,             // data index
        const float *norms,             // l2-norm of data points
        const float *points);           // data points 
};

} // end namespace ip
