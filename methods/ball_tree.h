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
//  Ball_Node: leaf and internal node structure of Ball_Tree
// -----------------------------------------------------------------------------
class Ball_Node {
public:
    int   n_;                       // number of data points
    int   d_;                       // dimension of data points
    int   k_max_;                   // max k value
    Ball_Node *lc_;                 // left  child
    Ball_Node *rc_;                 // right child
    
    float radius_;                  // radius of center and data points
    float norm_c_;                  // l2-norm of center
    float *center_;                 // the center of data points
    
    int   *index_;                  // data index (re-order for leaf only)
    float *data_;                   // data points (for leaf only)
    float *local_r_;                // local radius (for leaf only)
    
    float *x_cos_;                  // |x| cos(\theta) (for leaf only)
    float *x_sin_;                  // |x| sin(\theta) (for leaf only)
    float *lower_bounds_;           // lower bounds of data points
    float *node_lower_bounds_;      // lower bounds for this node
    
    // -------------------------------------------------------------------------
    Ball_Node(                      // constructor
        int   n,                        // number of data points
        int   d,                        // dimensionality
        bool  is_leaf,                  // is leaf node
        Ball_Node *lc,                  // left  child
        Ball_Node *rc,                  // right child
        int   *index,                   // data index
        const float *data);             // data points

    // -------------------------------------------------------------------------
    ~Ball_Node();                   // desctructor

    // -------------------------------------------------------------------------
    void kmips(                     // k-mips on ball node
        float ip,                       // inner product of center and query
        float norm_q,                   // l2-norm of input query
        const float *query,             // input query
        int   &cand,                    // # candidates to check (return)
        MaxK_List *list);               // k-mips results (return)

    // -------------------------------------------------------------------------
    float ball_upper_bound(         // upper bound for ball structure
        float ip,                       // inner product of query & center
        float norm_q);                  // l2-norm of query
    
    // -------------------------------------------------------------------------
    float ball_upper_bound(         // upper bound for ball structure
        int   i,                        // i-th point
        float ip,                       // inner product of query & center
        float norm_q);                  // l2-norm of query
    
    // -------------------------------------------------------------------------
    void linear_scan(               // linear scan
        float ip,                       // inner product for query and center
        float norm_q,                   // l2-norm of query
        const float *query,             // input query
        int   &cand,                    // # candidates to check (return)
        MaxK_List *list);               // k-mips results (return)
    
    // -------------------------------------------------------------------------
    float cone_upper_bound(         // upper bound for cone structure
        int   i,                        // i-th point
        float q_cos,                    // |q| cos(\phi) 
        float q_sin);                   // |q| sin(\phi)
    
    // -------------------------------------------------------------------------
    void traversal(                 // traversal ball-tree
        std::vector<Ball_Node*> &leaf); // leaves (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {
        u64 ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(float)*d_; // center_
        
        if (data_ == nullptr) { // internal node
            ret += lc_->get_estimated_memory();
            ret += rc_->get_estimated_memory();
        } else { // leaf node
            ret += sizeof(float)*n_*4;      // norms_, local_r_, x_cos_, x_sin_
            ret += sizeof(float)*n_*k_max_; // lower_bounds_
            ret += sizeof(float)*k_max_;    // node_lower_bounds_
        }
        return ret;
    }
};

// -----------------------------------------------------------------------------
//  Ball_Tree is a tree structure for k-Maximum Inner Product Search 
// -----------------------------------------------------------------------------
class Ball_Tree {
public:
    int   n_;                       // number of data points
    int   d_;                       // dimensionality
    int   leaf_size_;               // leaf size of ball-tree
    const float *data_;             // data points
    
    int   *index_;                  // data index
    Ball_Node *root_;               // root node of ball-tree
    
    // -------------------------------------------------------------------------
    Ball_Tree(                      // constructor
        int   n,                        // number of data points
        int   d,                        // dimensionality
        int   leaf_size,                // leaf size of ball-tree
        const float *data);             // data points
    
    // -------------------------------------------------------------------------
    ~Ball_Tree();                   // destructor
    
    // -------------------------------------------------------------------------
    void display();                 // display ball-tree
    
    // -------------------------------------------------------------------------
    int kmips(                      // k-mips on ball-tree
        int   k,                        // top-k value
        int   cand,                     // number of candidates
        const float *query,             // input query
        MaxK_List *list);               // k-mips results (return)
    
    // -------------------------------------------------------------------------
    void traversal(                 // traversal cone-tree to get leaf info
        std::vector<Ball_Node*> &leaf); // leaves (return)
    
    // -------------------------------------------------------------------------
    void traversal(                 // traversal cone-tree to get leaf info
        std::vector<Ball_Node*> &leaf,  // leaves (return)
        std::vector<int> &index);       // data index in a leaf order (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {
        u64 ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(int)*n_; // index_
        ret += root_->get_estimated_memory(); // memory of ball-nodes
        
        return ret;
    }
    
protected:
    // -------------------------------------------------------------------------
    Ball_Node* build(               // recursively build a ball-tree
        int n,                          // number of data points
        int *index);                    // data index (return)
    
    // -------------------------------------------------------------------------
    int find_furthest_id(           // find furthest data id
        int from,                       // input data id
        int n,                          // size of data index
        int *index);                    // data index
};

} // end namespace ip
