#include "h2_alsh.h"

namespace ip {

// -----------------------------------------------------------------------------
H2_ALSH::H2_ALSH(                   // constructor
    int   n,                            // data cardinality
    int   d,                            // dimensionality
    float b,                            // interval ratio for blocking data
    const float *data_set)              // data set
    : n_(n), d_(d), b_(b)
{
    gettimeofday(&g_start_time, nullptr);
    
    // 1. compute l2-norms & sort data_set in descending order of l2-norms
    data_index_ = new int[n];
    data_norms_ = new float[n];
    data_set_   = new float[(u64) n*d];
    compute_norm_and_sort(n, data_set, data_index_, data_norms_, data_set_);
    
    // 2. build blocks for data_set (with h2-trans) for batch pruning
    blocking_data_set(n, data_index_, data_norms_, data_set_);
    
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
    // compute l2-norms for input_set
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
void H2_ALSH::blocking_data_set(    // split the data_set into blocks
    int   n,                            // data cardinality
    const int   *data_index,            // data index
    const float *data_norms,            // data l2-norms
    const float *data_set)              // data set
{
    hashs_.clear();
    
    // split data_set into blocks and build qalsh for each block
    int start = 0;
    while (start < n) {
        // divide one block
        float M = data_norms[start];
        float min_radius = M * b_;
        int   cnt = 0, idx = start;
        
        while (idx < n && data_norms[idx] >= min_radius) {
            ++idx; ++cnt;
            if (cnt >= BLOCK_MAX_NUM) break;
        }
        // add one block by such cnt data points
        add_block_by_data_points(cnt, M, data_index+start, data_norms+start, 
            data_set + (u64)start*d_);
        
        // update parameters
        start += cnt;
    }
    assert(start == n);
}

// -----------------------------------------------------------------------------
void H2_ALSH::add_block_by_data_points(// add one block by data points
    int   n,                            // number of data points
    float M,                            // max l2-norm of data points
    const int   *index,                 // data index
    const float *norms,                 // l2-norm of data points
    const float *points)                // data points
{
    // init a block
    Block *block = new Block(n, M, index, norms, points);
    
    if (n > N_PTS_INDEX) {
        // build hash tables for qalsh
        block->lsh_ = new QALSH(n, d_+1, APPRX_RATIO_NNS);
        
        float *h2_data = new float[d_+1];
        QALSH *lsh = block->lsh_;
        Result *tables = lsh->tables_;
        
        int   m = lsh->m_;
        float M_sqr = M * M;
        for (int i = 0; i < n; ++i) {
            // construct new format of data by qnf transformation
            const float *data = points + (u64) i*d_;
            std::copy(data, data+d_, h2_data);
            h2_data[d_] = sqrt(M_sqr - SQR(norms[i]));
            
            // calc hash value for new format of data
            for (int j = 0; j < m; ++j) {
                float val = lsh->calc_hash_value(j, h2_data);
                tables[j*n+i].id_  = i;
                tables[j*n+i].key_ = val;
            }
        }
        for (int j = 0; j < m; ++j) {
            Result *table = tables + (u64) j*n;
            qsort(table, n, sizeof(Result), ResultComp);
        }
        delete[] h2_data;
    }
    // add this block
    hashs_.push_back(block);
}

// -----------------------------------------------------------------------------
H2_ALSH::~H2_ALSH()           // destructor
{
    for (auto hash : hashs_) { delete hash; hash = nullptr; }
    std::vector<Block*>().swap(hashs_);
    
    if (!data_index_) { delete[] data_index_; data_index_ = nullptr; }
    if (!data_norms_) { delete[] data_norms_; data_norms_ = nullptr; }
    if (!data_set_)   { delete[] data_set_;   data_set_   = nullptr; }
}

// -------------------------------------------------------------------------
void H2_ALSH::display()          // display parameters
{
    printf("Parameters of H2_ALSH:\n");
    printf("n        = %d\n", n_);
    printf("d        = %d\n", d_);
    printf("b        = %g\n", b_);
    printf("# blocks = %d\n\n", (int) hashs_.size());
    // for (int i = 0; i < hashs_.size(); ++i) {
    //     printf("%d (%g) ", hashs_[i]->n_, hashs_[i]->M_);
    //     if ((i+1) % 20 == 0) printf("\n");
    // }
    // printf("\n\n");
}

// -----------------------------------------------------------------------------
void H2_ALSH::kmips(                // k-mips
    int   k,                            // top-k value
    const float *query,                 // query vector
    MaxK_List *list)                    // k-mips result (return)
{
    gettimeofday(&g_start_time, nullptr);
    list->reset();
    
    // initialize parameters
    std::vector<float> h2_query(d_+1, 0.0f);
    std::vector<int> cand;
    
    float kip = MINREAL;
    float norm_q = sqrt(calc_inner_product(d_, query, query));
    
    // check data_set with blocks for batch pruning
    for (auto hash : hashs_) {
        int   n = hash->n_;
        float M = hash->M_;
        const int   *index = hash->index_;
        const float *norms = hash->norms_;
        const float *data  = hash->data_;
        
        // early pruning
        if (M * norm_q <= kip) break;
        
        // k-mips
        if (n > N_PTS_INDEX) {
            // get h2_query
            float lambda = M / norm_q;
            for (int j = 0; j < d_; ++j) h2_query[j] = lambda*query[j];
            h2_query[d_] = 0.0f;
            
            // perform k-nns by qalsh
            float range = sqrt(2.0f * (M*M - lambda*kip));
            hash->lsh_->knns(k, range, h2_query.data(), cand);
            
            // verify the candidates
            for (int id : cand) {
                if (norms[id] * norm_q > kip) {
                    const float *point = data + (u64) id*d_;
                    float ip = calc_inner_product(d_, point, query);
                    ++g_cand_cnt;
                    kip = list->insert(ip, index[id]+1);
                }
            }
        }
        else {
            // linear scan
            for (int j = 0; j < n; ++j) {
                if (norms[j]*norm_q <= kip) break;
                
                const float *point = data + (u64) j*d_;
                float ip = calc_inner_product(d_, point, query);
                ++g_cand_cnt;
                kip = list->insert(ip, index[j]+1);
            }
        }
    }
    gettimeofday(&g_end_time, nullptr);
    
    double query_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_run_time += query_time;
}

} // end namespace ip