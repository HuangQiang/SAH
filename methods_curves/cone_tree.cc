#include "cone_tree.h"

namespace ip {

// -----------------------------------------------------------------------------
//  Cone_Node: leaf and internal node structure of Cone_Tree
// -----------------------------------------------------------------------------
Cone_Node::Cone_Node(               // constructor
    int   n,                            // number of data points
    int   d,                            // dimension of data points
    bool  is_leaf,                      // is leaf node
    Cone_Node *lc,                      // left  child
    Cone_Node *rc,                      // right child
    int   *index,                       // data index
    const float *data)                  // data points
    : n_(n), d_(d), k_max_(-1), lc_(lc), rc_(rc), index_(index), 
    data_(nullptr), x_cos_(nullptr), x_sin_(nullptr), lower_bounds_(nullptr), 
    node_lower_bounds_(nullptr)
{
    M_cos_  = MAXREAL;
    M_sin_  = MINREAL;
    center_ = new float[d];
    if (is_leaf) {
        // init the local data by the input index and data
        data_ = new float[n*d];
        for (int i = 0; i < n; ++i) {
            const float *point = data + (u64) index[i]*d;
            float *new_point = data_ + (u64) i*d;
            std::copy(point, point+d, new_point);
        }
        // calc the center
        calc_centroid(n, d, data_, center_);
        norm_c_ = sqrt(calc_inner_product(d, center_, center_));
        
        // calc x_cos_ and x_sin_ of data points
        x_cos_ = new float[n];
        x_sin_ = new float[n];
        for (int i = 0; i < n; ++i) {
            const float *point = data_ + (u64) i*d;
            float x_cos = calc_inner_product(d, point, center_) / norm_c_;
            
            x_cos_[i] = x_cos;
            x_sin_[i] = sqrt(1.0f - SQR(x_cos));
            if (x_cos < M_cos_) M_cos_ = x_cos;
        }
        M_sin_ = sqrt(1.0f - SQR(M_cos_));
    }
    else {
        // calc the center
        int   lc_n = lc->n_, rc_n = rc->n_;
        float *lc_center = lc->center_;
        float *rc_center = rc->center_;
        for (int i = 0; i < d; ++i) {
            center_[i] = (lc_n*lc_center[i] + rc_n*rc_center[i]) / n;
        }
        norm_c_ = sqrt(calc_inner_product(d, center_, center_));
        
        // calc omega
        for (int i = 0; i < n; ++i) {
            const float *point = data + (u64) index[i]*d;
            float ip = calc_inner_product(d, point, center_);
            
            float x_cos = ip / norm_c_;
            if (x_cos < M_cos_) M_cos_ = x_cos;
        }
        M_sin_ = sqrt(1.0f - SQR(M_cos_));
    }
}

// -----------------------------------------------------------------------------
Cone_Node::~Cone_Node()             // destructor
{
    if (lc_ != nullptr) { delete lc_; lc_ = nullptr; }
    if (rc_ != nullptr) { delete rc_; rc_ = nullptr; }
    
    if (center_ != nullptr) { delete[] center_; center_ = nullptr; }
    if (data_   != nullptr) { delete[] data_;   data_   = nullptr; }
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
void Cone_Node::kmips(              // k-mips on cone node
    float ip,                           // inner product of center and query
    float norm_q,                       // l2-norm of input query
    const float *query,                 // input query
    int   &cand,                        // # candidates to check (return)
    MaxK_List *list)                    // k-mips results (return)
{
    // stop condition
    if (cand <= 0) return;

    float q_cos = ip / norm_c_;
    float q_sin = sqrt(SQR(norm_q) - SQR(q_cos));
    float ub  = est_upper_bound(q_cos, q_sin);
    if (ub <= list->min_key()) return;
    
    // kmips through the cone node
    if (data_ != nullptr) { // leaf node
        linear_scan(q_cos, q_sin, query, cand, list);
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
float Cone_Node::est_upper_bound(   // estimate upper bound for cone node
    float q_cos,                        // |q| cos(phi)
    float q_sin)                        // |q| sin(phi)
{
    return q_cos * M_cos_ + q_sin * M_sin_;
}

// -----------------------------------------------------------------------------
void Cone_Node::linear_scan(        // linear scan the data points
    float q_cos,                        // |q| cos(phi)
    float q_sin,                        // |q| sin(phi)
    const float *query,                 // input query
    int   &cand,                        // # candidates to check (return)
    MaxK_List *list)                    // k-mips results (return)
{
    float lambda = list->min_key();
    for (int i = 0; i < n_; ++i) {
        float ub = est_upper_bound(i, q_cos, q_sin);
        if (ub > lambda) {
            const float *point = data_ + (u64) i*d_;
            float ip = calc_inner_product(d_, point, query);
            ++g_ip_count;
            
            lambda = list->insert(ip, index_[i]+1);
        }
        // update candidate counter
        --cand; if (cand <= 0) break;
    }
}

// -----------------------------------------------------------------------------
float Cone_Node::est_upper_bound(   // estimate upper bound for data point
    int   i,                            // i-th point
    float q_cos,                        // |q| cos(phi)
    float q_sin)                        // |q| sin(phi)
{
    return q_cos * x_cos_[i] + q_sin * x_sin_[i];
}

// -----------------------------------------------------------------------------
void Cone_Node::traversal(          // traversal cone-tree
    std::vector<Cone_Node*> &leaf)      // leaves (return)
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
//  Cone_Tree is a tree structure for k-Maximum Inner Product Search 
// -----------------------------------------------------------------------------
Cone_Tree::Cone_Tree(               // constructor
    int   n,                            // number of data points
    int   d,                            // dimension of data points
    int   leaf_size,                    // leaf size of cone-tree
    const float *data)                  // data points
    : n_(n), d_(d), leaf_size_(leaf_size), data_(data)
{
    index_ = new int[n];
    int i = 0;
    std::iota(index_, index_+n, i++);
    
    root_ = build(n, index_);
}

// -----------------------------------------------------------------------------
Cone_Node* Cone_Tree::build(        // build a cone node
    int n,                              // numebr of data points
    int *index)                         // data index (return)
{
    Cone_Node* cur = nullptr;
    if (n <= leaf_size_) {
        // build leaf node
        cur = new Cone_Node(n, d_, true, nullptr, nullptr, index, data_);
    }
    else {
        // build internal node
        float *w = new float[d_];
        int   cnt = 0, left = 0, right = n-1;
        do {
            int x_p = rand() % n;
            int l_p = find_max_angle_id(x_p, n, index);
            int r_p = find_max_angle_id(l_p, n, index);
            assert(l_p != r_p);
            
            // note: we use l_p and r_p as two pivots
            const float *l_pivot = data_ + (u64) index[l_p]*d_;
            const float *r_pivot = data_ + (u64) index[r_p]*d_;
            for (int i = 0; i < d_; ++i) w[i] = l_pivot[i] - r_pivot[i];
    
            left = 0; right = n - 1;
            while (left <= right) {
                const float *x = data_ + (u64) index[left]*d_;
                float ip = calc_inner_product(d_, w, x);
                if (ip > 0) ++left;
                else { SWAP(index[left], index[right]); --right; }
            }
            ++cnt;
        } while ((left <= 0 || left >= n) && cnt <= 3);
        if (cnt > 3) left = n/2;
        delete[] w;
        
        Cone_Node* lc = build(left,   index);
        Cone_Node* rc = build(n-left, index+left);
        cur = new Cone_Node(n, d_, false, lc, rc, index, data_);
    }
    return cur;
}

// -----------------------------------------------------------------------------
int Cone_Tree::find_max_angle_id(   // find max angle id
    int from,                           // input data id
    int n,                              // size of data index
    int *index)                         // data index
{
    // as angle in [0,pi], ip=cos(angle) decreases as the angle increases
    const float *query = data_ + (u64) index[from]*d_;
    
    int max_angle_id = -1;  // max angle id
    float min_ip = MAXREAL; // corresponding max angle
    for (int i = 0; i < n; ++i) {
        if (i == from) continue;
        
        const float *point = data_ + (u64) index[i]*d_;
        float ip = calc_inner_product(d_, point, query);
        if (ip < min_ip) { min_ip = ip; max_angle_id = i; }
    }
    return max_angle_id;
}

// -----------------------------------------------------------------------------
Cone_Tree::~Cone_Tree()             // destructor
{
    if (index_ != nullptr) { delete[] index_; index_ = nullptr; }
    if (root_  != nullptr) { delete   root_;  root_  = nullptr; }
}

// -----------------------------------------------------------------------------
void Cone_Tree::display()           // display cone-tree
{    
    std::vector<Cone_Node*> leaf;
    root_->traversal(leaf);
    int num = (int) leaf.size();

    printf("Parameters of Cone_Tree:\n");
    printf("n         = %d\n", n_);
    printf("d         = %d\n", d_);
    printf("leaf_size = %d\n", leaf_size_);
    printf("# leaves  = %d\n", num);
    for (int i = 0; i < num; ++i) printf("%d ", leaf[i]->n_); printf("\n");
    printf("\n");
}

// -----------------------------------------------------------------------------
int Cone_Tree::kmips(               // k-mips on cone-tree
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
void Cone_Tree::traversal(          // traversal cone-tree to get leaf info
    std::vector<Cone_Node*> &leaf,      // leaves (return)
    std::vector<int> &index)            // data index in a leaf order (return)
{
    for (int i = 0; i < n_; ++i) index[i] = index_[i];
    
    root_->traversal(leaf);
}

// -----------------------------------------------------------------------------
void Cone_Tree::traversal(          // traversal cone-tree to get leaf info
    std::vector<Cone_Node*> &leaf)      // leaves (return)
{
    root_->traversal(leaf);
}

} // end namespace ip
