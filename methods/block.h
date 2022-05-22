#pragma once

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>

#include "def.h"
#include "pri_queue.h"
#include "util.h"
#include "qalsh.h"
#include "srp_lsh.h"

namespace ip {

// -----------------------------------------------------------------------------
//  Item_Block: Assistant Data Structure for H2_ALSH & SA_ALSH
// -----------------------------------------------------------------------------
class Item_Block {
public:
    int   n_;                       // number of items
    float M_;                       // max l2-norm of items
    float R_;                       // radius of shifted items
    const float *norms_;            // l2-norms of items
    const float *items_;            // items
    
    QALSH   *lsh_;                  // qalsh structure
    SRP_LSH *srp_;                  // srp-lsh structure
    
    // -------------------------------------------------------------------------
    Item_Block(                     // constructor
        int   n,                        // number of items
        float M,                        // max l2-norm of items
        const float *norms,             // l2-norms of items
        const float *items);            // items
    
    // -------------------------------------------------------------------------
    ~Item_Block();                  // destructor
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {    // get memory usage
        u64 ret = 0UL;
        ret += sizeof(*this);
        if (lsh_ != nullptr) ret += lsh_->get_estimated_memory();
        if (srp_ != nullptr) ret += srp_->get_estimated_memory();
        
        return ret;
    }
};

// -----------------------------------------------------------------------------
//  User_Block: Assistant Data Structure for H2_ALSH & SA_ALSH
// -----------------------------------------------------------------------------
class User_Block {
public:
    int   m_;                       // number of users
    int   k_max_;                   // max k value
    const int   *index_;            // index of users
    const float *norms_;            // l2-norms of users
    const float *users_;            // users
    const float *lower_bounds_;     // lower bounds of users
    float *block_lower_bounds_;     // block lower bounds
    
    // -------------------------------------------------------------------------
    User_Block(                     // constructor
        int   m,                        // number of users
        int   k_max,                    // max k value
        const int   *index,             // index of users
        const float *norms,             // l2-norms of users
        const float *users,             // users
        const float *lower_bounds);     // lower bounds of users
    
    // -------------------------------------------------------------------------
    ~User_Block();                  // destructor
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {    // get memory usage
        u64 ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(float)*k_max_; // block_lower_bounds_
        
        return ret;
    }
};

} // end namespace ip
