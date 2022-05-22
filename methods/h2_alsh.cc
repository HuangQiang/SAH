
#include "h2_alsh.h"

namespace ip {

// -----------------------------------------------------------------------------
H2_ALSH::H2_ALSH(                   // constructor
    int   n,                            // item cardinality
    int   m,                            // user cardinality
    int   d,                            // dimensionality
    float b,                            // interval ratio for blocking items
    const float *item_set,              // item set
    const float *user_set)              // user set
    : n_(n), m_(m), d_(d), b_(b), user_set_(user_set)
{
    gettimeofday(&g_start_time, nullptr);
    
    // 1. compute l2-norms & sort item_set in descending order of l2-norms
    item_index_ = new int[n];
    item_norms_ = new float[n];
    item_set_   = new float[(u64) n*d];
    compute_norm_and_sort(n, item_set, item_index_, item_norms_, item_set_);
    
    // 2. compute l2-norms for user_set
    user_norms_ = new float[m];
    for (int i = 0; i < m; ++i) {
        const float *user = user_set + (u64) i*d;
        user_norms_[i] = sqrt(calc_inner_product(d, user, user));
    }
    
    // 3. build blocks for the rest item_set (with h2-trans) for batch pruning
    blocking_item_set(n, item_norms_, item_set_);
    
    // get the pre-processing time and estimated memory
    gettimeofday(&g_end_time, nullptr);
    g_pre_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_memory = get_estimated_memory();
}

// -----------------------------------------------------------------------------
void H2_ALSH::compute_norm_and_sort(// compute l2-norm and sort (descending)
    int   n,                            // input set cardinality
    const float *input_set,             // input set
    int   *data_index,                  // index of sorted data (return)
    float *data_norms,                  // l2-norm of sorted data (return)
    float *data_set)                    // sorted data (return)
{
    // compute l2-norms for item_set
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
void H2_ALSH::blocking_item_set(    // split the rest item_set into blocks
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
void H2_ALSH::add_block_by_items(   // add one block by items
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
H2_ALSH::~H2_ALSH()                 // destructor
{
    for (auto hash : hashs_) { delete hash; hash = nullptr; }
    std::vector<Item_Block*>().swap(hashs_);
    
    if (!item_set_)     { delete[] item_set_;     item_set_     = nullptr; }
    if (!item_norms_)   { delete[] item_norms_;   item_norms_   = nullptr; }
    if (!item_index_)   { delete[] item_index_;   item_index_   = nullptr; }
    
    if (!user_norms_)   { delete[] user_norms_;   user_norms_   = nullptr; }
}

// -----------------------------------------------------------------------------
void H2_ALSH::display()             // display parameters
{
    printf("Parameters of H2_ALSH:\n");
    printf("n             = %d\n",   n_);
    printf("m             = %d\n",   m_);
    printf("d             = %d\n",   d_);
    printf("b             = %g\n",   b_);
    printf("# item blocks = %d\n\n", (int) hashs_.size());
    // for (int i = 0; i < hashs_.size(); ++i) {
    //     printf("%d (%g) ", hashs_[i]->n_, hashs_[i]->M_);
    //     if ((i+1) % 20 == 0) printf("\n");
    // }
    // printf("\n\n");
}

// -----------------------------------------------------------------------------
void H2_ALSH::reverse_kmips(        // reverse k-mips
    int   k,                            // top k value
    const float *query,                 // query vector
    std::vector<int> &result)           // reverse k-mips result (return)
{
    gettimeofday(&g_start_time, nullptr);
    std::vector<int>().swap(result);// clear space for result
    
    // compute l2-norm for query
    float query_norm = sqrt(calc_inner_product(d_, query, query)); 
    ++g_ip_count;
    
    // check each user in user_set
    float item_k_norm = item_norms_[k-1]; // k-th largest item norm
    MaxK_Array *arr = new MaxK_Array(k);
    
    for (int i = 0; i < m_; ++i) {
        // get user vector and its l2-norm
        const float *user = user_set_ + (u64) i*d_;
        float user_norm = user_norms_[i];
        
        float ip = calc_inner_product(d_, query, user); ++g_ip_count;
        float ub = user_norm * item_k_norm;
        if (ip >= ub) { 
            // add user id into the result of this query
            result.push_back(i); // Yes
        }
        else {
            // perform mips by h2-alsh
            arr->reset();
            if (kmips(k, ip, user_norm, user, arr) == 1) {
                result.push_back(i); // Yes
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
int H2_ALSH::kmips(                 // k-mips
    int   k,                            // top-k value
    float uq_ip,                        // inner product of user and query
    float user_norm,                    // l2-norm of input user
    const float *user,                  // input user
    MaxK_Array  *arr)                   // top-k mips array (return)
{
    // initialize parameters
    std::vector<float> h2_user(d_+1, 0.0f);
    std::vector<int> cand;
    float kip = arr->min_key();
    
    // check item_set with blocks for batch pruning
    for (auto hash : hashs_) {
        // early pruning (NOTE: as kip may NOT be true, 1 is not promising)
        float M = hash->M_;
        float ub = M * user_norm;
        if (ub <= uq_ip || ub <= kip) return 1; // Yes 
        
        // k-mips
        int   n = hash->n_;
        const float *norms = hash->norms_;
        const float *items = hash->items_;
        
        if (n > N_PTS_INDEX) {
            // get h2-user
            float lambda = M / user_norm;
            for (int j = 0; j < d_; ++j) h2_user[j] = lambda*user[j];
            h2_user[d_] = 0.0f;
            
            // perform knns by qalsh
            QALSH *lsh = hash->lsh_;
            float range  = sqrt(2.0f * (M*M - lambda*kip));
            lsh->knns(k, range, h2_user.data(), cand);
            
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
