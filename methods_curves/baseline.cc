#include "baseline.h"

namespace ip {

// -----------------------------------------------------------------------------
Scan::Scan(                         // constructor
    int   n,                            // item cardinality
    int   m,                            // user cardinality
    int   d,                            // dimensionality
    int   k_max,                        // max k value
    bool  parallel,                     // use openmp for parallel computing
    const float *item_set,              // item set
    const float *user_set)              // user set
    : m_(m), d_(d), k_max_(k_max), user_set_(user_set)
{
    gettimeofday(&g_start_time, nullptr);
    
    // compute l2-norm for user_set
    user_norms_ = new float[m_];
    for (int i = 0; i < m_; ++i) {
        const float *user = user_set_ + (u64) i*d_;
        user_norms_[i] = sqrt(calc_inner_product(d_, user, user));
    }
    
    // compute l2-norm sort item_set in descending order by their l2-norms
    float *item_norms = new float[n];   // l2-norm of item vectors
    float *items = new float[(u64) n*d];
    compute_norm_and_sort(n, item_set, item_norms, items);
    
    // compute k bounds for user_set
    k_bounds_ = new float[(u64) m*k_max];
    if (parallel) {
        printf("run openmp\n");
        parallel_k_bounds_computation(n, item_norms, items);
    } else {
        k_bounds_computation(n, item_norms, items);
    }
    // release space
    delete[] item_norms;
    delete[] items;
    
    // get the pre-processing time and estimated memory
    gettimeofday(&g_end_time, nullptr);
    g_pre_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_memory = get_estimated_memory();
}

// -----------------------------------------------------------------------------
void Scan::compute_norm_and_sort(   // compute l2-norm and sort (descending)
    int   n,                            // input set cardinality
    const float *input_set,             // input set
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
        int id = ret[i].id_;
        data_norms[i] = ret[i].key_;
        
        const float *data = input_set + (u64) id*d_;
        std::copy(data, data + d_, data_set + (u64)i*d_);
    }
    delete[] ret;
}

// -----------------------------------------------------------------------------
void Scan::parallel_k_bounds_computation(// compute k bounds for user_set
    int   n,                            // item cardinality
    const float *item_norms,            // l2-norm of items
    const float *item_set)              // item set
{
    MaxK_Array **arr = new MaxK_Array*[m_];
    for (int i = 0; i < m_; ++i) arr[i] = new MaxK_Array(k_max_);
    
    #pragma omp parallel num_threads(THREAD_NUM)
    {
        #pragma omp for
        for (int i = 0; i < m_; ++i) {
            kmips(k_max_, n, user_norms_[i], user_set_ + (u64) i*d_, item_norms,
                item_set, arr[i], k_bounds_ + (u64)i*k_max_);
        }
    }
    
    // release space
    for (int i = 0; i < m_; ++i) { delete arr[i]; arr[i] = nullptr; }
    delete[] arr; arr = nullptr;
}

// -----------------------------------------------------------------------------
void Scan::kmips(                   // k-mips by linear scan with pruning
    int   k,                        // top-k value
    int   n,                        // item cardinality
    float user_norm,                // l2-norm of user
    const float *user,              // user vector
    const float *item_norms,        // l2-norm of sorted items
    const float *item_set,          // sorted items
    MaxK_Array *arr,                // top-k array (return)
    float *k_bounds)                // k bounds (return)
{
    arr->reset();
    
    // find k-mips for this user over the item_set
    float tau = MINREAL; // k-th maximum ip value
    for (int j = 0; j < n; ++j) {
        // leverage the descending order of item_norms for pruning
        float upper_bound = user_norm*item_norms[j];
        if (upper_bound < tau) break;
        
        const float *item = item_set + (u64) j*d_;
        float ip = calc_inner_product(d_, user, item);
        tau = arr->add(ip);
    }
    update_k_bounds(k, arr, k_bounds);
}

// -----------------------------------------------------------------------------
void Scan::update_k_bounds(         // update lower bound
    int   k,                            // top-k value
    MaxK_Array *arr,                    // top-k array
    float *k_bounds)                    // k bounds (return)
{
    float *keys = arr->keys_;
    std::copy(keys, keys+k, k_bounds);
}

// -----------------------------------------------------------------------------
void Scan::k_bounds_computation(    // compute k bounds for user_set
    int   n,                            // item cardinality
    const float *item_norms,            // l2-norm of items
    const float *item_set)              // item set
{
    MaxK_Array *arr = new MaxK_Array(k_max_);
    for (int i = 0; i < m_; ++i) {
        kmips(k_max_, n, user_norms_[i], user_set_ + (u64) i*d_, item_norms,
            item_set, arr, k_bounds_ + (u64)i*k_max_);
    }
    delete arr;
}

// -----------------------------------------------------------------------------
Scan::~Scan()                       // destructor
{
    if (!user_norms_) { delete[] user_norms_; user_norms_ = nullptr; }
    if (!k_bounds_)   { delete[] k_bounds_;   k_bounds_   = nullptr; }
}

// -----------------------------------------------------------------------------
void Scan::display()                // display parameters
{
    printf("Parameters of Scan:\n");
    printf("m             = %d\n", m_);
    printf("d             = %d\n", d_);
    printf("k_max         = %d\n", k_max_);
    printf("indexing time = %g Seconds\n", g_pre_time);
    printf("est. memory   = %g MB\n", g_memory / 1048576.0);
    printf("\n");
}

// -----------------------------------------------------------------------------
void Scan::reverse_kmips(           // reverse k-mips
    int   k,                            // top k value
    const float *query,                 // query vector
    std::vector<int> &result)           // reverse k-mips result (return)
{
    gettimeofday(&g_start_time, nullptr);
    
    // clear space for result
    std::vector<int>().swap(result);
    assert(k > 0 && k <= k_max_);
    
    // compute l2-norm for query
    float query_norm = sqrt(calc_inner_product(d_, query, query));
    ++g_ip_count;
    
    // sequential scan each user
    for (int i = 0; i < m_; ++i) {
        float tau = k_bounds_[i*k_max_+k-1]; // get the exact k-th mip
        float ip = calc_inner_product(d_, query, user_set_+(u64)i*d_);
        ++g_ip_count;
        
        if (ip >= tau) result.push_back(i);
    }
    gettimeofday(&g_end_time, nullptr);
    
    double query_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_run_time += query_time;
}

} // end namespace ip
