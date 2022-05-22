#pragma once

#include <iostream>
#include <algorithm>
#include <cassert>
#include <vector>

#include "def.h"
#include "util.h"
#include "pri_queue.h"

namespace ip {

// -----------------------------------------------------------------------------
//  Sign-Random Projection LSH (SRP_LSH) is used to solve the problem of 
//  c-Approximate Maximum Cosine Similarity Search (c-AMCSS)
// 
//  the idea was introduced by Moses S. Charikar in his paper "Similarity 
//  estimation techniques from rounding algorithms", In Proceedings of the 
//  thiry-fourth annual ACM symposium on Theory of computing (STOC), pages 
//  380–388, 2002.
// -----------------------------------------------------------------------------
class SRP_LSH {
public:
    int   n_;                       // number of data objects
    int   d_;                       // dimensionality
    int   K_;                       // number of hash functions
    int   m_;                       // number of compressed uint64_t hash code
    // bool  align_;                   // align or not
    
    float *proj_;                   // random projection vectors
    u64   *hash_keys_;              // hash code of data objects
    u32   *table16_;                // table to record the number of "1" bits
    
    // -------------------------------------------------------------------------
    SRP_LSH(                        // constructor
        int n,                          // number of data objects
        int d,                          // dimensionality
        int K);                         // number of hash functions
    
    // -------------------------------------------------------------------------
    ~SRP_LSH();                     // destructor
    
    // -------------------------------------------------------------------------
    bool calc_hash_code(            // calc hash code after random projection
        int   id,                       // projection vector id
        const float *data);             // input data
    
    // -------------------------------------------------------------------------
    void compress_hash_code(        // compress hash code with 64 bits
        const bool *hash_code,          // input hash code
        u64   *hash_key);               // hash key (return)
    
    // -------------------------------------------------------------------------
    void display();                 // display parameters
    
    // -------------------------------------------------------------------------
    int kmcss(                      // k-mcss
        int   k,                        // top-k value
        const float *query,             // input query
        std::vector<int> &cand);        // k-mcss candidates (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {    // get memory usage
        u64 ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(float)*K_*d_;   // proj_
        ret += sizeof(int)*(1 << 16); // table16_
        ret += sizeof(u64)*n_*m_;;    // hash_key_
        return ret;
    }

protected:
    // -------------------------------------------------------------------------
    u32 bit_count(u32 x) {          // count the number of 1 bits of x
        u32 num = x - ((x >> 1) & 033333333333) - ((x >> 2) & 011111111111);
        return ((num + (num >> 3)) & 030707070707) % 63;
    }
    
    // -------------------------------------------------------------------------
    u32 table_lookup(u64 x) {       // table lookup the match value
        return table16_[x & 0xffff] + table16_[(x>>16) & 0xffff] + 
            table16_[(x>>32) & 0xffff] + table16_[(x>>48) & 0xffff];
    }
};

} // end namespace ip
