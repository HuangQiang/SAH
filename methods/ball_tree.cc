#include "ball_tree.h"

namespace ip {

// -----------------------------------------------------------------------------
//  Ball_Node: leaf and internal node structure of Ball_Tree
// -----------------------------------------------------------------------------
Ball_Node::Ball_Node(               // constructor
    int   n,                            // number of data points
    int   d,                            // data dimension
    bool  is_leaf,                      // is leaf node
    Ball_Node *lc,                      // left  child
    Ball_Node *rc,                      // right child
    int   *index,                       // data index
    const float *data)                  // data points
    : n_(n), d_(d), k_max_(-1), lc_(lc), rc_(rc), index_(index), 
    data_(nullptr), local_r_(nullptr), x_cos_(nullptr), x_sin_(nullptr), 
    lower_bounds_(nullptr), node_lower_bounds_(nullptr)
{
    radius_ = MINREAL;
    center_ = new float[d];
    if (is_leaf) {
        // calc center_, norm_c_ and radius_ based on local data
        calc_centroid(n, d, index, data, center_);
        norm_c_ = sqrt(calc_inner_product(d, center_, center_));
        
        Result *result = new Result[n];
        for (int i = 0; i < n; ++i) {
            const float *point = data + (u64) index[i]*d;
            result[i].id_  = index[i];
            result[i].key_ = calc_l2_dist(d, point, center_);
        }
        qsort(result, n, sizeof(Result), ResultCompDesc);
        radius_ = result[0].key_;
        
        // alloc space for parameters
        data_    = new float[(u64) n*d];
        x_cos_   = new float[n]; 
        x_sin_   = new float[n];
        local_r_ = new float[n];
        for (int i = 0; i < n; ++i) {
            // init index_ & local_r_
            index_[i]   = result[i].id_;
            local_r_[i] = result[i].key_;
            
            // init data_
            const float *point = data + (u64) index_[i]*d;
            std::copy(point, point+d, data_+(u64)i*d);
            
            // init x_cos_ & x_sin_
            float ip = calc_inner_product(d, point, center_);
            x_cos_[i] = ip / norm_c_;
            x_sin_[i] = sqrt(1.0f - SQR(x_cos_[i]));
        }
        delete[] result;
    }
    else {
        // calc center_ & norm_c_ based on its two children
        int   lc_n = lc_->n_, rc_n = rc_->n_;
        float *lc_center = lc_->center_;
        float *rc_center = rc_->center_;
        for (int i = 0; i < d; ++i) {
            center_[i] =  (lc_n*lc_center[i] + rc_n*rc_center[i]) / n;
        }
        norm_c_ = sqrt(calc_inner_product(d, center_, center_));
        
        // init radius_
        radius_ = -1.0f;
        for (int i = 0; i < n; ++i) {
            const float *point = data + (u64) index[i]*d;
            float dist = calc_l2_sqr(d, point, center_);
            if (dist > radius_) radius_ = dist;
        }
        radius_ = sqrt(radius_);
    }
}

// -----------------------------------------------------------------------------
Ball_Node::~Ball_Node()             // desctructor
{
    if (lc_ != nullptr) { delete lc_; lc_ = nullptr; }
    if (rc_ != nullptr) { delete rc_; rc_ = nullptr; }
    
    if (center_ != nullptr) { delete[] center_; center_ = nullptr; }
    if (data_   != nullptr) { delete[] data_;   data_   = nullptr; }
    if (local_r_!= nullptr) { delete[] local_r_;local_r_= nullptr; }
    
    if (x_cos_  != nullptr) { delete[] x_cos_;  x_cos_  = nullptr; }
    if (x_sin_  != nullptr) { delete[] x_sin_;  x_sin_  = nullptr; }
    
    if (lower_bounds_ != nullptr) { 
        delete[] lower_bounds_; lower_bounds_ = nullptr;
    }
    if (node_lower_bounds_ != nullptr) { 
        delete[] node_lower_bounds_; node_lower_bounds_ = nullptr;
    }
}

// -----------------------------------------------------------------------------
void Ball_Node::kmips(              // k-mips on cone node
    float ip,                           // inner product of center and query
    float norm_q,                       // l2-norm of input query
    const float *query,                 // input query
    int   &cand,                        // # candidates to check (return)
    MaxK_List *list)                    // k-mips results (return)
{
    // early stop 1
    if (cand <= 0) return;
    
    // early stop 2
    float ub = ball_upper_bound(ip, norm_q);
    if (ub <= list->min_key()) return;
    
    // kmips through the cone node
    if (data_ != nullptr) { // leaf node
        linear_scan(ip, norm_q, query, cand, list);
    }
    else { // internal node
        // center preference 
        float lc_ip = calc_inner_product(d_, lc_->center_, query);
        float rc_ip = (ip*n_ - lc_ip*lc_->n_) / rc_->n_; 
        ++g_ip_count;
        
        if (lc_ip > rc_ip) {
            lc_->kmips(lc_ip, norm_q, query, cand, list);
            rc_->kmips(rc_ip, norm_q, query, cand, list);
        }
        else {
            rc_->kmips(rc_ip, norm_q, query, cand, list);
            lc_->kmips(lc_ip, norm_q, query, cand, list);
        }
    }
}

// -----------------------------------------------------------------------------
float Ball_Node::ball_upper_bound(  // upper bound for ball structure
    float ip,                           // inner product of query & center
    float norm_q)                       // l2-norm of query
{
    return ip + norm_q * radius_;
}

// -----------------------------------------------------------------------------
float Ball_Node::ball_upper_bound(  // upper bound for ball structure
    int   i,                            // i-th point
    float ip,                           // inner product of query & center
    float norm_q)                       // l2-norm of query
{
    return ip + norm_q * local_r_[i];
}

// -----------------------------------------------------------------------------
void Ball_Node::linear_scan(        // linear scan
    float ip,                           // inner product for query and center
    float norm_q,                       // l2-norm of query
    const float *query,                 // input query
    int   &cand,                        // # candidates to check (return)
    MaxK_List *list)                    // k-mips results (return)
{
    float q_cos = ip / norm_c_;
    float q_sin = sqrt(SQR(norm_q) - SQR(q_cos));
    
    float lambda = list->min_key();
    for (int i = 0; i < n_; ++i) {
        // calc point-level ball upper bound 
        float ub = ball_upper_bound(i, ip, norm_q);
        if (ub <= lambda) return;
        
        // calc point-level cone upper bound 
        ub = cone_upper_bound(i, q_cos, q_sin);
        if (ub > lambda) {
            const float *point = data_ + (u64) i*d_;
            float ip = calc_inner_product(d_, point, query);
            ++g_ip_count;
            
            lambda = list->insert(ip, index_[i]+1); 
        }
        // update candidate counter
        --cand; if (cand <= 0) return;
    }
}

// -----------------------------------------------------------------------------
float Ball_Node::cone_upper_bound(  // upper bound for cone structure
    int   i,                            // i-th point
    float q_cos,                        // |q| cos(\phi) 
    float q_sin)                        // |q| sin(\phi)
{
    return q_cos * x_cos_[i] + q_sin * x_sin_[i];
}

// -----------------------------------------------------------------------------
void Ball_Node::traversal(          // traversal ball-tree
    std::vector<Ball_Node*> &leaf)      // leaves (return)
{
    if (data_ != nullptr) {
        leaf.push_back(this);
    } 
    else {
        lc_->traversal(leaf);
        rc_->traversal(leaf);
    }
}

// -----------------------------------------------------------------------------
//  Ball_Tree is a tree structure for k-Maximum Inner Product Search 
// -----------------------------------------------------------------------------
Ball_Tree::Ball_Tree(               // constructor
    int   n,                            // number of data points
    int   d,                            // dimension of data points
    int   leaf_size,                    // leaf size of ball-tree
    const float *data)                  // data points
    : n_(n), d_(d), leaf_size_(leaf_size), data_(data)
{
    index_ = new int[n];
    int i = 0;
    std::iota(index_, index_+n, i++);
    
    root_ = build(n, index_);
}

// -----------------------------------------------------------------------------
Ball_Node* Ball_Tree::build(        // recursively build a ball-tree
    int n,                              // number of data points
    int *index)                         // data index (return)
{
    Ball_Node *cur = nullptr;
    if (n <= leaf_size_) {
        // build leaf node
        cur = new Ball_Node(n, d_, true, nullptr, nullptr, index, data_);
    }
    else {
        // build internal node
        float *w = new float[d_];
        int   cnt = 0, left = 0, right = n-1;
        do {
            int x_p = rand() % n;
            int l_p = find_furthest_id(x_p, n, index);
            int r_p = find_furthest_id(l_p, n, index);
            assert(l_p != r_p);
            
            // note: we use l_p and r_p as two pivots
            const float *l_pivot = data_ + (u64) index[l_p]*d_;
            const float *r_pivot = data_ + (u64) index[r_p]*d_;
            float l_sqr = 0.0f, r_sqr = 0.0f;
            for (int i = 0; i < d_; ++i) {
                float l_v = l_pivot[i];
                float r_v = r_pivot[i];
                
                w[i] = r_v-l_v; l_sqr += SQR(l_v); r_sqr += SQR(r_v);
            }
            float b = (l_sqr - r_sqr) / 2;
    
            left = 0, right = n-1;
            while (left <= right) {
                const float *x = data_ + (u64) index[left]*d_;
                float val = calc_inner_product(d_, w, x) + b;
                if (val < 0.0f) ++left;
                else { SWAP(index[left], index[right]); --right; }
            }
            ++cnt;
        } while ((left <= 0 || left >= n) && (cnt <= 3)); // ensure split into 2
        if (cnt > 3) left = n/2; // meet the case that cannot split, force split
        delete[] w;

        Ball_Node *lc = build(left,   index);
        Ball_Node *rc = build(n-left, index+left);
        cur = new Ball_Node(n, d_, false, lc, rc, index, data_);
    }
    return cur;
}

// -----------------------------------------------------------------------------
int Ball_Tree::find_furthest_id(    // find furthest data id
    int from,                           // input id
    int n,                              // number of data index
    int *index)                         // data index
{
    const float *query = data_ + (u64) index[from]*d_;
    
    int   far_id   = -1;
    float far_dist = -1.0f;
    for (int i = 0; i < n; ++i) {
        if (i == from) continue;
        
        const float *point = data_ + (u64) index[i]*d_;
        float dist = calc_l2_sqr(d_, point, query);
        if (far_dist < dist) { far_dist = dist; far_id = i; }
    }
    return far_id;
}

// -----------------------------------------------------------------------------
Ball_Tree::~Ball_Tree()          // desctructor
{
    if (root_  != nullptr) { delete   root_;  root_  = nullptr; }
    if (index_ != nullptr) { delete[] index_; index_ = nullptr; }
}

// -----------------------------------------------------------------------------
void Ball_Tree::display()      // display bc-tree
{
    std::vector<Ball_Node*> leaf;
    root_->traversal(leaf);
    int num = (int) leaf.size();
        
    printf("Parameters of Ball_Tree:\n");
    printf("n         = %d\n", n_);
    printf("d         = %d\n", d_);
    printf("leaf_size = %d\n", leaf_size_);
    printf("# leaves  = %d\n", num);
    for (int i = 0; i < num; ++i) printf("%d ", leaf[i]->n_); printf("\n");
    printf("\n");
}

// -----------------------------------------------------------------------------
int Ball_Tree::kmips(               // k-mips on cone tree
    int   k,                            // top-k value
    int   cand,                         // number of candidates
    const float *query,                 // input query
    MaxK_List *list)                    // k-mips results (return)
{
    cand = std::min(cand+k-1, n_);
    
    float norm_q = sqrt(calc_inner_product(d_, query, query));
    float ip = calc_inner_product(d_, root_->center_, query);
    ++g_ip_count;
    
    int total = cand;
    root_->kmips(ip, norm_q, query, cand, list);
    return total - cand;
}

// -----------------------------------------------------------------------------
void Ball_Tree::traversal(          // traversal ball-tree to get leaf info
    std::vector<Ball_Node*> &leaf,      // leaves (return)
    std::vector<int> &index)            // data index in a leaf order (return)
{
    for (int i = 0; i < n_; ++i) index[i] = index_[i];
    
    root_->traversal(leaf);
}

// -----------------------------------------------------------------------------
void Ball_Tree::traversal(          // traversal ball-tree to get leaf info
    std::vector<Ball_Node*> &leaf)      // leaves (return)
{
    root_->traversal(leaf);
}

} // end namespace ip
