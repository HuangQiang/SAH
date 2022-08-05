
#include "sa_simpfer.h"

namespace ip {

// -----------------------------------------------------------------------------
SA_Simpfer::SA_Simpfer(             // constructor
    int   n,                            // item cardinality
    int   m,                            // user cardinality
    int   d,                            // dimensionality
    int   k_max,                        // max k value
    int   K,                            // # hash tables for SRP-LSH
    float b,                            // interval ratio for blocking itemss
    const float *item_set,              // item set
    const float *user_set)              // user set
    : n_(n), m_(m), d_(d), k_max_(k_max), K_(K), b_(b)
{
    gettimeofday(&g_start_time, nullptr);
    
    // 1. compute l2-norms & sort item_set in descending order of l2-norms
    item_index_ = new int[n];
    item_norms_ = new float[n];
    item_set_   = new float[(u64) n*d];
    compute_norm_and_sort(n, item_set, item_index_, item_norms_, item_set_);
    
    // 2. compute l2-norms & sort user_set in descending order of l2-norms
    user_index_ = new int[m];
    user_norms_ = new float[m];
    user_set_   = new float[(u64) m*d];
    compute_norm_and_sort(m, user_set, user_index_, user_norms_, user_set_);
    
    // 3. determine k_max approximate mips results as lower bounds for user_set
    int n0 = k_max*COEFF; // only consider the first n0 elements in item_set
    if (n0 > n) n0 = n;
    
    lower_bounds_ = new float[(u64) m*k_max];
    lower_bounds_computation(n0);
    
    // 4. build blocks for user_set for batch pruning
    blocking_user_set();
    
    // 5. build blocks for the rest item_set (with sa-trans) for batch pruning
    blocking_item_set(n-n0, item_norms_+n0, item_set_+(u64)n0*d);
    
    // get the pre-processing time and estimated memory
    gettimeofday(&g_end_time, nullptr);
    g_pre_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_memory = get_estimated_memory();
}

// -----------------------------------------------------------------------------
void SA_Simpfer::compute_norm_and_sort(// compute l2-norm and sort (descending)
    int   n,                            // input set cardinality
    const float *input_set,             // input set
    int   *data_index,                  // index of sorted data (return)
    float *data_norms,                  // l2-norm of sorted data (return)
    float *data_set)                    // sorted data (return)
{
    // compute l2-norm for input_set
    Result *ret = new Result[n];
    for (int i = 0; i < n; ++i) {
        const float *data = input_set + (u64) i*d_;
        ret[i].id_  = i;
        ret[i].key_ = sqrt(calc_inner_product(d_, data, data));
    }
    // sort the l2-norm in descending order
    qsort(ret, n, sizeof(Result), ResultCompDesc);
    
    // init data_index, data_norms, data_set
    for (int i = 0; i < n; ++i) {
        data_index[i] = ret[i].id_;
        data_norms[i] = ret[i].key_;
        
        const float *data = input_set + (u64) data_index[i]*d_;
        std::copy(data, data + d_, data_set + (u64)i*d_);
    }
    delete[] ret;
}

// -----------------------------------------------------------------------------
void SA_Simpfer::lower_bounds_computation(// compute lower bounds for user_set
    int n0)                             // the first n0 elements in item_set
{
    MaxK_Array *arr = new MaxK_Array(k_max_);
    for (int i = 0; i < m_; ++i) {
        // get user vector and its l2-norm
        const float *user = user_set_ + (u64) i*d_;
        float user_norm = user_norms_[i];
        
        // find k-mips for this user over the item_set_
        float tau = MINREAL; // k-th maximum ip value
        arr->reset();
        for (int j = 0; j < n0; ++j) {
            // leverage the descending order of item norms for pruning
            float upper_bound = user_norm*item_norms_[j];
            if (tau > upper_bound) break;
            
            const float *item = item_set_ + (u64) j*d_;
            float ip = calc_inner_product(d_, user, item);
            tau = arr->add(ip);
        }
        update_lower_bound(k_max_, arr, lower_bounds_ + (u64)i*k_max_);
    }
    delete arr;
}

// -----------------------------------------------------------------------------
void SA_Simpfer::update_lower_bound(// update lower bound
    int   k,                            // top-k value
    MaxK_Array *arr,                    // top-k array
    float *lower_bound)                 // lower bound (return)
{
    float *keys = arr->keys_;
    std::copy(keys, keys+k, lower_bound);
}

// -----------------------------------------------------------------------------
void SA_Simpfer::blocking_user_set()// split the user_set into blocks
{
    // clear blocks & calc block size
    blocks_.clear();
    block_size_ = (int) ceil(log2((double)m_)*20.0);
    if (block_size_ > m_) block_size_ = m_;
    
    // split the user_set into blocks
    int block_size = block_size_;
    for (int i = 0; i < m_; i += block_size) {
        // get the block size for this block
        if (i+block_size > m_) block_size = m_-i;
        
        // add a new block
        User_Block *block = new User_Block(block_size, k_max_, user_index_+i,
            user_norms_+i, user_set_+(u64)i*d_, lower_bounds_+(u64)i*k_max_);
        blocks_.push_back(block);
    }
}

// -----------------------------------------------------------------------------
void SA_Simpfer::blocking_item_set( // split the rest item_set into blocks
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
void SA_Simpfer::add_block_by_items(// add one block by items
    int   n,                            // number of items
    float M,                            // max l2-norm of items
    const float *norms,                 // l2-norm of items
    const float *items)                 // items
{
    // init a block
    Item_Block *block = new Item_Block(n, M, norms, items);
    
    if (n > N_PTS_INDEX) {
        float *sa_item = new float[d_+1];
        
        // calc the centroid of items
        float *centroid = new float[d_];
        calc_centroid(n, d_, items, centroid);
        
        // calc the shifted items (by the centroid) & their l2-norm squares
        float *shift_items = new float[n*d_];
        float *shift_norms = new float[n];
        float R = shift_data_and_norms(n, d_, items, centroid, shift_items, 
            shift_norms);
        block->R_ = sqrt(R);
        
        // build hash tables for srp-lsh
        block->srp_ = new SRP_LSH(n, d_+1, K_);
        
        SRP_LSH *srp = block->srp_;
        bool *hash_code = new bool[K_];
        u64  *hash_keys = srp->hash_keys_;
        int  m = srp->m_;
        for (int i = 0; i < n; ++i) {
            // construct new format of data by qnf transformation
            const float *shift_item = shift_items + (u64) i*d_;
            std::copy(shift_item, shift_item+d_, sa_item);
            sa_item[d_] = sqrt(R - shift_norms[i]);
            
            // calc hash value for new format of data
            for (int j = 0; j < K_; ++j) {
                hash_code[j] = srp->calc_hash_code(j, sa_item);
            }
            srp->compress_hash_code(hash_code, hash_keys + (u64)i*m);
        }
        delete[] hash_code;
        delete[] centroid;
        delete[] shift_norms;
        delete[] shift_items;
        delete[] sa_item;
    }
    // add this block
    hashs_.push_back(block);
}

// -----------------------------------------------------------------------------
SA_Simpfer::~SA_Simpfer()           // destructor
{
    for (auto hash : hashs_) { delete hash; hash = nullptr; }
    std::vector<Item_Block*>().swap(hashs_);
    
    if (!item_set_)     { delete[] item_set_;     item_set_     = nullptr; }
    if (!item_norms_)   { delete[] item_norms_;   item_norms_   = nullptr; }
    if (!item_index_)   { delete[] item_index_;   item_index_   = nullptr; }
    
    for (auto block : blocks_) { delete block; block = nullptr; }
    std::vector<User_Block*>().swap(blocks_);
    
    if (!user_set_)     { delete[] user_set_;     user_set_     = nullptr; }
    if (!user_norms_)   { delete[] user_norms_;   user_norms_   = nullptr; }
    if (!user_index_)   { delete[] user_index_;   user_index_   = nullptr; }
    if (!lower_bounds_) { delete[] lower_bounds_; lower_bounds_ = nullptr; }
}

// -------------------------------------------------------------------------
void SA_Simpfer::display()          // display parameters
{
    printf("Parameters of SA_Simpfer:\n");
    printf("n             = %d\n",   n_);
    printf("m             = %d\n",   m_);
    printf("d             = %d\n",   d_);
    printf("k_max         = %d\n",   k_max_);
    printf("K             = %d\n",   K_);
    printf("b             = %g\n",   b_);
    printf("block_size    = %d\n",   block_size_);
    printf("# user blocks = %d\n",   (int) blocks_.size());
    printf("# item blocks = %d\n\n", (int) hashs_.size());
    // for (int i = 0; i < hashs_.size(); ++i) {
    //     printf("%d (%g) ", hashs_[i]->n_, hashs_[i]->R_);
    //     if ((i+1) % 20 == 0) printf("\n");
    // }
    // printf("\n\n");
}

// -----------------------------------------------------------------------------
void SA_Simpfer::reverse_kmips(     // reverse k-mips
    int   k,                            // top k value
    const float *query,                 // query vector
    std::vector<int> &result)           // reverse k-mips result (return)
{
    gettimeofday(&g_start_time, nullptr);
    std::vector<int>().swap(result);// clear space for result
    assert(k > 0 && k <= k_max_);   // validate the range of k
    
    // compute l2-norm for query
    float query_norm = sqrt(calc_inner_product(d_, query, query)); 
    ++g_ip_count;
    
    // check user_set with blocks for batch pruning
    float item_k_norm = item_norms_[k-1]; // k-th largest item norm
    MaxK_Array *arr = new MaxK_Array(k);
    
    for (auto block : blocks_) {
        // lemma 3
        float ub = query_norm * block->norms_[0];
        if (ub < block->block_lower_bounds_[k-1]) continue;
        
        // get user statistics from this block
        int   m = block->m_;
        const int   *user_index   = block->index_;
        const float *user_norms   = block->norms_;
        const float *user_set     = block->users_;
        const float *lower_bounds = block->lower_bounds_;
        
        for (int i = 0; i < m; ++i) {
            // get the lower bound for this user
            const float *lower_bound = lower_bounds + (u64) i*k_max_;
            float user_norm = user_norms[i];
            
            // 1.1 as <q,u> <= |q|*|u|, use lower_buond for pruning (lemma 1*)
            ub = query_norm * user_norm; 
            if (ub < lower_bound[k-1]) continue; // No
            
            // 1.2 use lower_bound for pruning  (lemma 1)
            const float *user = user_set + (u64) i*d_;
            float ip = calc_inner_product(d_, query, user); ++g_ip_count;
            if (ip < lower_bound[k-1]) continue; // No
            
            // 2. use item upper bound for pruning (lemma 2)
            ub = user_norm * item_k_norm;
            if (ip >= ub) { 
                // add user id into the result of this query
                result.push_back(user_index[i]); // Yes
            }
            else {
                // init the top-k array from the lower bound of this user
                arr->init(k, lower_bound);
                arr->add(ip);
                if (kmips(k, ip, user_norm, user, arr) == 1) {
                    result.push_back(user_index[i]); // Yes
                }
            }
        }
    }
    delete arr;
    gettimeofday(&g_end_time, nullptr);
    
    double query_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_run_time += query_time;
}

// -----------------------------------------------------------------------------
int SA_Simpfer::kmips(              // k-mips
    int   k,                            // top-k value
    float uq_ip,                        // inner product of user and query
    float user_norm,                    // l2-norm of input user
    const float *user,                  // input user
    MaxK_Array  *arr)                   // top-k mips array (return)
{
    // initialize parameters
    std::vector<float> sa_user(d_+1, 0.0f);
    std::vector<int> cand;
    float kip = arr->min_key();
    
    // check item_set with blocks for batch pruning
    for (auto hash : hashs_) {
        // early pruning (NOTE: as kip may NOT be  true, 1 is not promising)
        float M = hash->M_;
        float ub = M * user_norm;
        if (ub <= uq_ip || ub <= kip) return 1; // Yes 
        
        // k-mips
        int   n = hash->n_;
        const float *norms = hash->norms_;
        const float *items = hash->items_;
        
        if (n > N_PTS_INDEX) {
            // get sa-user
            float lambda = hash->R_ / user_norm;
            for (int j = 0; j < d_; ++j) sa_user[j] = lambda*user[j];
            sa_user[d_] = 0.0f;
            
            // perform knns by srp-lsh
            SRP_LSH *srp = hash->srp_;
            srp->kmcss(k, sa_user.data(), cand);
            
            // verify the candidates
            for (int id : cand) {
                // note that the id is NOT sorted in descending order
                if (norms[id] * user_norm >= kip) {
                    const float *item = items + (u64) id*d_;
                    float ip = calc_inner_product(d_, item, user);
                    ++g_ip_count;
                    
                    kip = arr->add(ip);
                    if (kip > uq_ip) return 0; // return No
                }
            }
        }
        else {
            // linear scan
            for (int j = 0; j < n; ++j) {
                // NOTE: since kip may NOT be the true, 1 is not promising
                ub = norms[j] * user_norm;
                if (ub <= uq_ip || ub <= kip) return 1; // Yes 
                
                const float *item = items + (u64) j*d_;
                float ip = calc_inner_product(d_, item, user);
                ++g_ip_count;
                
                kip = arr->add(ip);
                if (kip > uq_ip) return 0; // return No
            }
        }
    }
    return 1;
}

} // end namespace ip
