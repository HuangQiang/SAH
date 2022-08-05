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
//  Cone_Node: leaf and internal node structure of Cone_Tree
// -----------------------------------------------------------------------------
class Cone_Node {
public:
    int   n_;                       // number of data points
    int   d_;                       // dimensionality
    int   k_max_;                   // max k value
    Cone_Node *lc_;                 // left  child
    Cone_Node *rc_;                 // right child
    
    float M_cos_;                   // M cos(omega)
    float M_sin_;                   // M sin(omega)
    float norm_c_;                  // l2-norm of center
    float *center_;                 // the center of data points
    
    int   *index_;                  // data index
    float *data_;                   // data points (only for leaf)
    
    float *x_cos_;                  // x cos(angle) of center and data (only for leaf)
    float *x_sin_;                  // x sin(angle) of center and data (only for leaf)
    float *lower_bounds_;           // lower bounds of data points
    float *node_lower_bounds_;      // lower bounds for this node
    
    // -------------------------------------------------------------------------
    Cone_Node(                      // constructor
        int   n,                    // number of data points
        int   d,                    // dimensionality
        bool  is_leaf,              // is leaf node
        Cone_Node* lc,              // left  child
        Cone_Node* rc,              // right child
        int   *index,               // data index
        const float *data);         // data points
    
    // -------------------------------------------------------------------------
    ~Cone_Node();                   // destructor 
    
    // -------------------------------------------------------------------------
    void kmips(                     // k-mips on cone node
        float ip,                       // inner product of center and query
        float norm_q,                   // l2-norm of input query
        const float *query,             // input query
        int   &cand,                    // # candidates to check (return)
        MaxK_List *list);               // k-mips results (return)
    
    // -------------------------------------------------------------------------
    void traversal(                 // traversal cone-tree
        std::vector<Cone_Node*> &leaf); // leaves (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {
        u64 ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(float)*d_; // center_
        
        if (data_ == nullptr) { // internal node
            ret += lc_->get_estimated_memory();
            ret += rc_->get_estimated_memory();
        } else { // leaf node
            ret += sizeof(float)*n_*3;      // norms_, x_cos_, & x_sin_
            ret += sizeof(float)*n_*k_max_; // lower_bounds_
            ret += sizeof(float)*k_max_;    // node_lower_bounds_
        }
        return ret;
    }
    
    // -------------------------------------------------------------------------
    float est_upper_bound(          // estimate upper bound for cone node
        float q_cos,                    // |q| cos(phi)
        float q_sin);                   // |q| sin(phi)
    
    // -------------------------------------------------------------------------
    void linear_scan(               // linear scan
        float q_cos,                    // |q| cos(phi)
        float q_sin,                    // |q| sin(phi)
        const float *query,             // input query
        int   &cand,                    // # candidates to check (return)
        MaxK_List *list);               // k-mips results (return)
    
    // -------------------------------------------------------------------------
    float est_upper_bound(          // estimate upper bound for data point
        int   i,                        // i-th point
        float q_cos,                    // |q| cos(\phi) 
        float q_sin);                   // |q| sin(\phi)
};

// -----------------------------------------------------------------------------
//  Cone_Tree is a tree structure for k-Maximum Inner Product Search 
// -----------------------------------------------------------------------------
class Cone_Tree {
public:
    int   n_;                       // number of data points
    int   d_;                       // dimensionality
    int   leaf_size_;               // leaf size of cone-tree
    const float *data_;             // data points
    
    int   *index_;                  // data index
    Cone_Node *root_;               // the root node of cone-tree
    
    // -------------------------------------------------------------------------
    Cone_Tree(                      // constructor
        int   n,                        // number of data points
        int   d,                        // dimension of data points
        int   leaf_size,                // leaf size of cone-tree
        const float *data);             // data points
    
    // -------------------------------------------------------------------------
    ~Cone_Tree();                   // destructor
    
    // -------------------------------------------------------------------------
    void display();                 // display cone-tree
    
    // -------------------------------------------------------------------------
    int kmips(                      // k-mips on cone-tree
        int   k,                        // top-k value
        int   cand,                     // number of candidates
        const float *query,             // input query
        MaxK_List *list);               // k-mips results (return)
    
    // -------------------------------------------------------------------------
    void traversal(                 // traversal cone-tree to get leaf info
        std::vector<Cone_Node*> &leaf); // leaves (return)
    
    // -------------------------------------------------------------------------
    void traversal(                 // traversal cone-tree to get leaf info
        std::vector<Cone_Node*> &leaf,  // leaves (return)
        std::vector<int> &index);       // data index in a leaf order (return)
    
    // -------------------------------------------------------------------------
    u64 get_estimated_memory() {
        u64 ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(int)*n_; // index_
        ret += root_->get_estimated_memory(); // memory of cone nodes
        
        return ret;
    }
    
protected:
    // -------------------------------------------------------------------------
    Cone_Node* build(               // build a cone node
        int n,                          // numebr of data points
        int *index);                    // data index (return)
    
    // -------------------------------------------------------------------------
    int find_max_angle_id(          // find max angle id
        int from,                       // input data id
        int n,                          // size of data index
        int *index);                    // data index
};

} // end namespace ip
