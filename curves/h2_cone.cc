
#include "h2_cone.h"

namespace ip {

// -----------------------------------------------------------------------------
H2_CONE::H2_CONE(                   // constructor
    int   n,                            // item cardinality
    int   m,                            // user cardinality
    int   d,                            // dimensionality
    int   k_max,                        // max k value
    int   leaf,                         // leaf size of cone-tree
    float b,                            // interval ratio for blocking items
    const float *item_set,              // item set
    const float *user_set)              // user set
    : n_(n), m_(m), d_(d), k_max_(k_max), leaf_(leaf), b_(b)
{
    gettimeofday(&g_start_time, nullptr);
    
    // 1. compute l2-norms & sort item_set in descending order of l2-norms
    item_index_ = new int[n];
    item_norms_ = new float[n];
    item_set_   = new float[(u64) n*d];
    compute_norm_and_sort(item_set);
    
    // 2. build blocks (with cone-tree) for user_set for batch pruning
    int n0 = k_max*COEFF; // only consider the first n0 elements in item_set
    if (n0 > n) n0 = n;   // keep at most n
    
    blocking_user_set(n0, user_set);
    
    // 3. build blocks for the rest item_set (with sa-trans) for batch pruning
    blocking_item_set(n-n0, item_norms_+n0, item_set_+(u64)n0*d);
    
    // get the pre-processing time and estimated memory
    gettimeofday(&g_end_time, nullptr);
    g_pre_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_memory = get_estimated_memory();
}

// -----------------------------------------------------------------------------
void H2_CONE::compute_norm_and_sort(// compute l2-norm and sort (descending)
    const float *item_set)              // item_set
{
    // compute l2-norms for item_set
    Result *ret = new Result[n_];
    for (int i = 0; i < n_; ++i) {
        const float *item = item_set + (u64) i*d_;
        ret[i].id_  = i;
        ret[i].key_ = sqrt(calc_inner_product(d_, item, item));
    }
    // sort the l2-norms in descending order
    qsort(ret, n_, sizeof(Result), ResultCompDesc);
    
    // init item_index_, item_norms_, and item_set_
    for (int i = 0; i < n_; ++i) {
        item_index_[i] = ret[i].id_;
        item_norms_[i] = ret[i].key_;
        
        const float *item = item_set + (u64) item_index_[i]*d_;
        float *new_item = item_set_ + (u64) i*d_;
        std::copy(item, item + d_, new_item);
    }
    delete[] ret;
}

// -----------------------------------------------------------------------------
void H2_CONE::blocking_user_set(    // split the user_set into blocks
    int   n0,                           // the first n0 elements in item_set
    const float *user_set)              // user_set
{
    // build a cone-tree for user_set
    tree_ = new Cone_Tree(m_, d_, leaf_, user_set);
    
    // traversal the cone-tree to get the blocks (cone-nodes) of user_set 
    blocks_.clear();
    tree_->traversal(blocks_);
    
    // build lower bounds for the users in each cone-node
    for (auto block : blocks_) {
        int   m = block->n_; // number of users 
        const float *user_set = block->data_;
        
        block->k_max_ = k_max_;
        block->lower_bounds_ = new float[m*k_max_];
        block->node_lower_bounds_ = new float[k_max_];
        
        // compute lower bounds for the users
        lower_bounds_computation(m, n0, user_set, block->lower_bounds_);
        
        // compute lower bounds for this cone-node
        node_lower_bounds_computation(m, block->lower_bounds_,
            block->node_lower_bounds_);
    }
}

// -----------------------------------------------------------------------------
void H2_CONE::lower_bounds_computation(// compute lower bounds for users
    int   m,                            // number of users
    int   n0,                           // the first n0 elements in item_set
    const float *user_set,              // users
    float *lower_bounds)                // lower bounds (return)
{
    MaxK_Array *arr = new MaxK_Array(k_max_);
    for (int i = 0; i < m; ++i) {
        // get user vector and its l2-norm
        const float *user = user_set + (u64) i*d_;
        
        // find k-mips for this user over item_set_
        float tau = MINREAL; // k-th maximum ip value
        arr->reset();
        for (int j = 0; j < n0; ++j) {
            // leverage the descending order of item norms for pruning
            if (tau > item_norms_[j]) break;
            
            const float *item = item_set_ + (u64) j*d_;
            float ip = calc_inner_product(d_, user, item);
            tau = arr->add(ip);
        }
        update_lower_bound(k_max_, arr, lower_bounds+(u64)i*k_max_);
    }
    delete arr;
}

// -----------------------------------------------------------------------------
void H2_CONE::update_lower_bound(   // update lower bound
    int   k,                            // top-k value
    MaxK_Array *arr,                    // top-k array
    float *lower_bound)                 // lower bound (return)
{
    float *keys = arr->keys_;
    std::copy(keys, keys+k, lower_bound);
}

// -----------------------------------------------------------------------------
void H2_CONE::node_lower_bounds_computation(// compute lower bound for a node
    int   m,                            // number of users
    const float *lower_bounds,          // lower bounds
    float *node_lower_bounds)           // node lower bounds (return)
{
    // init node lower bounds
    for (int i = 0; i < k_max_; ++i) node_lower_bounds[i] = MAXREAL;
    
    // update node lower bounds
    for (int i = 0; i < m; ++i) {
        const float *lb = lower_bounds + (u64) i*k_max_;
        for (int j = 0; j < k_max_; ++j) {
            if (node_lower_bounds[j] > lb[j]) node_lower_bounds[j] = lb[j];
        }
    }
}

// -----------------------------------------------------------------------------
void H2_CONE::blocking_item_set(    // split the rest item_set into blocks
    int   n,                            // item cardinality
    const float *item_norms,            // item l2-norms
    const float *item_set)              // item set
{
    hashs_.clear();
    
    // split item_set into blocks and build qalsh for each block
    int start = 0;
    while (start < n) {
        // divide one block
        float M = item_norms[start];
        float min_radius = M * b_;
        int   cnt = 0, idx = start;
        
        while (idx < n && item_norms[idx] >= min_radius) {
            ++idx; ++cnt;
            if (cnt >= BLOCK_MAX_NUM) break;
        }
        // add one block by such cnt items
        add_block_by_items(cnt, M, item_norms+start, item_set+(u64)start*d_);
        
        // update parameters
        start += cnt;
    }
    assert(start == n);
}

// -----------------------------------------------------------------------------
void H2_CONE::add_block_by_items(   // add one block by items
    int   n,                            // number of items
    float M,                            // max l2-norm of items
    const float *norms,                 // l2-norm of items
    const float *items)                 // items
{
    // init a block
    Item_Block *block = new Item_Block(n, M, norms, items);
    
    if (n > N_PTS_INDEX) {
        float M_sqr = M * M;
        float *h2_item = new float[d_+1];
        
        // build hash tables for qalsh
        block->lsh_ = new QALSH(n, d_+1, APPRX_RATIO_NNS);
        
        QALSH *lsh = block->lsh_;
        int m = lsh->m_;
        Result *tables = lsh->tables_;
        for (int i = 0; i < n; ++i) {
            // construct new format of data by qnf transformation
            const float *item = items + (u64) i*d_;
            std::copy(item, item+d_, h2_item);
            h2_item[d_] = sqrt(M_sqr - SQR(norms[i]));
            
            // calc hash value for new format of data
            for (int j = 0; j < m; ++j) {
                float val = lsh->calc_hash_value(j, h2_item);
                tables[j*n+i].id_  = i;
                tables[j*n+i].key_ = val;
            }
        }
        for (int j = 0; j < m; ++j) {
            Result *table = tables + (u64) j*n;
            qsort(table, n, sizeof(Result), ResultComp);
        }
        delete[] h2_item;
    }
    // add this block
    hashs_.push_back(block);
}

// -----------------------------------------------------------------------------
H2_CONE::~H2_CONE()                 // destructor
{
    for (auto hash : hashs_) { delete hash; hash = nullptr; }
    std::vector<Item_Block*>().swap(hashs_);
    
    if (!item_set_)   { delete[] item_set_;   item_set_   = nullptr; }
    if (!item_norms_) { delete[] item_norms_; item_norms_ = nullptr; }
    if (!item_index_) { delete[] item_index_; item_index_ = nullptr; }
    
    std::vector<Cone_Node*>().swap(blocks_);
    
    if (tree_ != nullptr) { delete tree_; tree_ = nullptr; }
}

// -------------------------------------------------------------------------
void H2_CONE::display()             // display parameters
{
    printf("Parameters of H2_CONE:\n");
    printf("n             = %d\n",   n_);
    printf("m             = %d\n",   m_);
    printf("d             = %d\n",   d_);
    printf("k_max         = %d\n",   k_max_);
    printf("leaf          = %d\n",   leaf_);
    printf("b             = %g\n",   b_);
    printf("# user blocks = %d\n",   (int) blocks_.size());
    printf("# item blocks = %d\n\n", (int) hashs_.size());
    // for (int i = 0; i < hashs_.size(); ++i) {
    //     printf("%d (%g) ", hashs_[i]->n_, hashs_[i]->M_);
    //     if ((i+1) % 20 == 0) printf("\n");
    // }
    // printf("\n\n");
}

// -----------------------------------------------------------------------------
void H2_CONE::reverse_kmips(        // reverse k-mips
    int   k,                            // top k value
    int   cand,                         // # candidates
    const float *query,                 // query vector
    std::vector<int> &result)           // reverse k-mips result (return)
{
    gettimeofday(&g_start_time, nullptr);
    std::vector<int>().swap(result);// clear space for result
    assert(k > 0 && k <= k_max_);   // validate the range of k
    cand_cnt_ = 0;                  // init candidate counter
    
    // compute l2-norm for query
    float query_norm = sqrt(calc_inner_product(d_, query, query)); 
    ++g_ip_count;
    
    // check user_set
    float item_k_norm = item_norms_[k-1]; // k-th largest item norm
    MaxK_Array *arr = new MaxK_Array(k);
    
    for (auto block : blocks_) {
        // lemma 3
        float block_k_lb = block->node_lower_bounds_[k-1];
        if (query_norm < block_k_lb) continue;
        
        // New Lemma: use node upper bound for batch pruning
        float ip = calc_inner_product(d_, query, block->center_); ++g_ip_count;
        float q_cos = ip / block->norm_c_;
        float q_sin = sqrt(SQR(query_norm) - SQR(q_cos));
        
        float ub = block->est_upper_bound(q_cos, q_sin);
        if (ub < block_k_lb) continue;
        
        // get user statistics from this block
        int   m = block->n_;
        const int   *user_index   = block->index_;
        const float *user_set     = block->data_;
        const float *lower_bounds = block->lower_bounds_;
        
        for (int i = 0; i < m; ++i) {
            // get the lower bound for this user
            const float *lower_bound = lower_bounds + (u64) i*k_max_;
            float user_k_lb = lower_bound[k-1];
            
            // 1.1 New Lemma: use point (user) upper bound for pruning
            ub = block->est_upper_bound(i, q_cos, q_sin);
            if (ub < user_k_lb) continue; // No
            
            // 1.2 use lower_bound for pruning  (lemma 1)
            const float *user = user_set + (u64) i*d_;
            ip = calc_inner_product(d_, query, user); ++g_ip_count;
            if (ip < user_k_lb) continue; // No
            
            // 2. use item upper bound for pruning (lemma 2)
            if (ip >= item_k_norm) { 
                // add user id into the result of this query
                result.push_back(user_index[i]); // Yes
            }
            else {
                // init the top-k array from the lower bound of this user
                arr->init(k, lower_bound);
                arr->add(ip);
                if (kmips(k, cand, ip, user, arr) == 1) {
                    result.push_back(user_index[i]); // Yes
                }
            }
            if (cand_cnt_ >= cand) break;
        }
        if (cand_cnt_ >= cand) break;
    }
    delete arr;
    gettimeofday(&g_end_time, nullptr);
    
    double query_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_run_time += query_time;
}

// -----------------------------------------------------------------------------
int H2_CONE::kmips(                 // k-mips
    int   k,                            // top-k value
    int   cand,                         // # candidates
    float uq_ip,                        // inner product of user and query
    const float *user,                  // input user
    MaxK_Array  *arr)                   // top-k mips array (return)
{
    // initialize parameters
    std::vector<float> h2_user(d_+1, 0.0f);
    std::vector<int> cand_list;
    float kip = arr->min_key();
    
    // check item_set with blocks for batch pruning
    for (auto hash : hashs_) {
        // early pruning (NOTE: as kip may NOT be true, 1 is not promising)
        float M = hash->M_;
        float ub = M; // user_norm = 1.0
        if (ub <= uq_ip || ub <= kip) return 1; // Yes 
        
        // k-mips
        int   n = hash->n_;
        const float *norms = hash->norms_;
        const float *items = hash->items_;
        
        if (n > N_PTS_INDEX) {
            // get h2-user
            float lambda = M; // user_norm = 1.0
            for (int j = 0; j < d_; ++j) h2_user[j] = lambda*user[j];
            h2_user[d_] = 0.0f;
            
            // perform knns by qalsh
            QALSH *lsh = hash->lsh_;
            float range = sqrt(2.0f * (M*M - lambda*kip));
            lsh->knns(k, range, h2_user.data(), cand_list);
            
            // verify the candidates
            for (int id : cand_list) {
                // note that the id is NOT sorted in descending order
                if (norms[id] >= kip) { // user_norm = 1.0
                    const float *item = items + (u64) id*d_;
                    float ip = calc_inner_product(d_, item, user);
                    ++g_ip_count;
                    kip = arr->add(ip);
                    if (kip > uq_ip) return 0; // return No
                    
                    ++cand_cnt_;
                    if (cand_cnt_ >= cand) return 0;
                }
            }
        }
        else {
            // linear scan
            for (int j = 0; j < n; ++j) {
                // NOTE: since kip may NOT be the true, 1 is not promising
                ub = norms[j]; // user_norm = 1.0
                if (ub <= uq_ip || ub <= kip) return 1; // Yes 
                
                const float *item = items + (u64) j*d_;
                float ip = calc_inner_product(d_, item, user);
                ++g_ip_count;
                kip = arr->add(ip);
                if (kip > uq_ip) return 0; // return No
                
                ++cand_cnt_;
                if (cand_cnt_ >= cand) return 0;
            }
        }
        if (cand_cnt_ >= cand) return 0;
    }
    return 1;
}

} // end namespace ip
