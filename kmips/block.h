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
//  Block: Assistant Data Structure for H2_ALSH & SA_ALSH
// -----------------------------------------------------------------------------
class Block {
public:
    int   n_;                       // number of data
    float M_;                       // max l2-norm of data
    float R_;                       // radius of shifted data points
    const int   *index_;            // data index
    const float *norms_;            // data l2-norms
    const float *data_;             // data points
    
    QALSH   *lsh_;                  // qalsh structure
    SRP_LSH *srp_;                  // srp-lsh structure
    
    // -------------------------------------------------------------------------
    Block(                          // constructor
        int   n,                        // number of data
        float M,                        // max l2-norm of data
        const int   *index,             // data index
        const float *norms,             // data l2-norms
        const float *data);             // data points
    
    // -------------------------------------------------------------------------
    ~Block();                       // destructor
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {    // get memory usage
        u64 ret = 0UL;
        ret += sizeof(*this);
        if (lsh_ != nullptr) ret += lsh_->get_estimated_memory();
        if (srp_ != nullptr) ret += srp_->get_estimated_memory();
        
        return ret;
    }
};

} // end namespace ip
