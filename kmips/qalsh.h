#pragma once

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>
#include <vector>

#include "def.h"
#include "pri_queue.h"
#include "util.h"

namespace ip {

// -----------------------------------------------------------------------------
//  Query-Aware Locality-Sensitive Hashing (QALSH) is used to solve the problem 
//  of c-Approximate Nearest Neighbor (c-ANN) search.
//
//  the idea was introduced by Qiang Huang, Jianlin Feng, Yikai Zhang, Qiong 
//  Fang, and Wilfred Ng in their paper "Query-aware locality-sensitive hashing 
//  for approximate nearest neighbor search", in Proceedings of the VLDB 
//  Endowment (PVLDB), 9(1), pages 1–12, 2015.
// -----------------------------------------------------------------------------
class QALSH {
public:
    int    n_;                      // number of data points
    int    d_;                      // dimensionality
    float  c0_;                     // approximation ratio
    float  w_;                      // bucket width
    int    m_;                      // number of hash tables
    int    l_;                      // collision threshold
    float  *a_;                     // lsh functions
    Result *tables_;                // hash tables
    
    // -------------------------------------------------------------------------
    QALSH(                          // constructor
        int   n,                        // number of data points
        int   d,                        // dimensionality
        float c0);                      // approximation ratio
    
    // -------------------------------------------------------------------------
    ~QALSH();                       // destructor
    
    // -------------------------------------------------------------------------
    float calc_hash_value(          // calc hash value
        int   tid,                      // table id
        const float *data);             // input data
    
    // -------------------------------------------------------------------------
    void display();                 // display parameters
    
    // -------------------------------------------------------------------------
    int knns(                       // approximate k-nns
        int   k,                        // top-k value
        float R,                        // limited search range
        const float *query,             // input query
        std::vector<int> &cand);        // candidates (return)
    
    // -------------------------------------------------------------------------
    int knns(                       // approximate k-nns
        int   k,                        // top-k value
        const float *query,             // input query
        std::vector<int> &cand);        // candidates (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {    // get estimated memory usage
        u64 ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(float)*m_*d_; // a_
        ret += sizeof(Result)*m_*n_;// tables_
        return ret;
    }

protected: 
    // assistant parameters for fast k-NN search
    int   *freq_;                   // collision frequency for n data points
    bool  *checked_;                // checked or not for n data points
    bool  *b_flag_;                 // bucket flag for m hash tables
    bool  *r_flag_;                 // range  flag for m hash tables
    int   *l_pos_;                  // left  positions for m hash tables
    int   *r_pos_;                  // right positions for m hash tables
    float *q_val_;                  // m hash values of query
    
    // -------------------------------------------------------------------------
    float calc_p(float x) {         // calc collision probability, x = w/(2*r)
        return new_cdf(x, 0.001f);      // cdf of [-x, x]
    }
    // -------------------------------------------------------------------------
    void alloc();                   // alloc space for assistant parameters
    
    // -------------------------------------------------------------------------
    void init_position(             // init left/right positions
        const float *query);            // input query
    
    // -------------------------------------------------------------------------
    int dynamic_collsion_counting(  // dynamic collision counting
        int   k,                        // top-k value
        float R,                        // limited search range
        std::vector<int> &cand);        // candidates (return)
    
    // -------------------------------------------------------------------------
    int dynamic_collsion_counting(  // dynamic collision counting
        int   k,                        // top-k value
        std::vector<int> &cand);        // candidates (return)
    
    // -------------------------------------------------------------------------
    void free();                    // free space for assistant parameters
};

} // end namespace ip
