#include "qalsh.h"

namespace ip {

// -----------------------------------------------------------------------------
QALSH::QALSH(                       // constructor
    int   n,                            // number of data objects
    int   d,                            // dimension of data objects
    float c0)                           // approximation ratio
    : n_(n), d_(d), c0_(c0)
{
    // init parameters
    w_ = sqrt((8.0f*c0*c0*log(c0)) / (c0*c0-1.0f));
    
    float p1    = calc_p(w_ / 2.0f);
    float p2    = calc_p(w_ / (2.0f*c0));
    float beta  = (float) CANDIDATES / (float) n;
    float delta = 1.0f / E;
    
    float para1 = sqrt(log(2.0f / beta));
    float para2 = sqrt(log(1.0f / delta));
    float para3 = 2.0f * (p1 - p2) * (p1 - p2);
    float eta   = para1 / para2;
    float alpha = (eta * p1 + p2) / (1.0f + eta);
    
    m_ = (int) ceil((para1 + para2) * (para1 + para2) / para3);
    l_ = (int) ceil(alpha * m_);

    // generate hash functions, chosen from N(0.0, 1.0)
    a_ = new float[m_*d_];
    for (int i = 0; i < m_*d_; ++i) a_[i] = gaussian(0.0F, 1.0F);

    // allocate space for hash tables
    tables_ = new Result[(u64) m_*n_];
}

// -----------------------------------------------------------------------------
QALSH::~QALSH()                     // destructor
{
    if (a_      != nullptr) { delete[] a_;      a_      = nullptr; }
    if (tables_ != nullptr) { delete[] tables_; tables_ = nullptr; }
}

// -----------------------------------------------------------------------------
float QALSH::calc_hash_value(       // calc hash value
    int   tid,                          // table id
    const float *data)                  // input data
{
    return calc_inner_product(d_, a_+tid*d_, data);
}

// -----------------------------------------------------------------------------
void QALSH::display()               // display parameters
{
    printf("Parameters of QALSH:\n");
    printf("    n  = %d\n",   n_);
    printf("    d  = %d\n",   d_);
    printf("    c0 = %.1f\n", c0_);
    printf("    w  = %f\n",   w_);
    printf("    m  = %d\n",   m_);
    printf("    l  = %d\n",   l_);
    printf("\n");
}

// -----------------------------------------------------------------------------
int QALSH::knns(                    // approximate k-nns
    int   k,                            // top-k value
    float R,                            // limited search range
    const float *query,                 // input query
    std::vector<int> &cand)             // candidates (return)
{
    cand.clear();
    
    // dynamic collsion counting
    alloc();
    init_position(query);
    int cand_cnt = dynamic_collsion_counting(k, R, cand);
    free();

    return cand_cnt;
}

// -----------------------------------------------------------------------------
int QALSH::knns(                    // approximate k-nns
    int   k,                            // top-k value
    const float *query,                 // input query
    std::vector<int> &cand)             // candidates (return)
{
    cand.clear();
    
    // dynamic collsion counting
    alloc();
    init_position(query);
    int cand_cnt = dynamic_collsion_counting(k, cand);
    free();

    return cand_cnt;
}

// -----------------------------------------------------------------------------
void QALSH::alloc()                 // alloc space for assistant parameters
{
    freq_    = new int[n_];  
    checked_ = new bool[n_]; 
    b_flag_  = new bool[m_]; 
    r_flag_  = new bool[m_]; 
    l_pos_   = new int[m_];
    r_pos_   = new int[m_];
    q_val_   = new float[m_];

    memset(freq_,    0,     sizeof(int)*n_);
    memset(checked_, false, sizeof(bool)*n_);
    memset(b_flag_,  true,  sizeof(bool)*m_);
    memset(r_flag_,  true,  sizeof(bool)*m_);
}

// -----------------------------------------------------------------------------
void QALSH::init_position(          // init left/right positions
    const float *query)                 // input query
{
    Result tmp;
    for (int i = 0; i < m_; ++i) {
        q_val_[i] = calc_hash_value(i, query);
        tmp.key_ = q_val_[i];
        
        Result *table = tables_ + (u64) i*n_;
        int pos = std::lower_bound(table, table+n_, tmp, cmp) - table;
        if (pos <= 0) { 
            l_pos_[i] = -1;  r_pos_[i] = 0;
        } 
        else if (pos >= n_-1) {
            l_pos_[i] = n_-1; r_pos_[i] = n_;
        }
        else { 
            l_pos_[i] = pos; r_pos_[i] = pos + 1;
        }
    }
}

// -----------------------------------------------------------------------------
int QALSH::dynamic_collsion_counting(// dynamic collision counting
    int   k,                            // top-k value
    float R,                            // limited search range
    std::vector<int> &cand)             // candidates (return)
{
    // k-nn search via dynamic collision counting
    int   cand_num  = CANDIDATES + k - 1; // total candidate number
    int   cand_cnt  = 0;            // candidate counter
    int   num_range = 0;            // number of search range flag
    int   num_bucket, cnt, pos, id;
    
    float radius = 1.0f;            // search radius
    float width  = radius * w_ / 2.0f;  // bucket width
    float range  = R > MAXREAL-1.0f ? MAXREAL : R * w_ / 2.0f; // search range
    float q_v, ldist, rdist;
    
    while (true) {
        // ---------------------------------------------------------------------
        // step 1: initialize the stop condition for current round
        num_bucket = 0; memset(b_flag_, true, sizeof(bool)*m_);
        
        // ---------------------------------------------------------------------
        // step 2: (R,c)-NN search
        while (num_bucket < m_ && num_range < m_) {
            for (int j = 0; j < m_; ++j) {
                if (!b_flag_[j]) continue;
                
                Result *table = tables_ + (u64)j*n_;
                q_v = q_val_[j], ldist = -1.0f, rdist = -1.0f;
                
                // -------------------------------------------------------------
                // step 2.1: scan the left part of hash table
                cnt = 0; pos = l_pos_[j];
                while (cnt < SCAN_SIZE) {
                    ldist = MAXREAL;
                    if (pos >= 0) ldist = fabs(q_v - table[pos].key_);
                    else break;
                    if (ldist > width || ldist > range) break;
                    
                    id = table[pos].id_; ++freq_[id];
                    if (freq_[id] >= l_ && !checked_[id]) {
                        checked_[id] = true;
                        cand.push_back(id);
                        if (++cand_cnt >= cand_num) break;
                    }
                    --pos; ++cnt;
                }
                if (cand_cnt >= cand_num) break;
                l_pos_[j] = pos;
                
                // -------------------------------------------------------------
                // step 2.2: scan the right part of hash table
                cnt = 0; pos = r_pos_[j];
                while (cnt < SCAN_SIZE) {
                    rdist = MAXREAL;
                    if (pos < n_) rdist = fabs(q_v - table[pos].key_);
                    else break;
                    if (rdist > width || rdist > range) break;
                    
                    id = table[pos].id_; ++freq_[id];
                    if (freq_[id] >= l_ && !checked_[id]) {
                        checked_[id] = true;
                        cand.push_back(id);
                        if (++cand_cnt >= cand_num) break;
                    }
                    ++pos; ++cnt;
                }
                if (cand_cnt >= cand_num) break;
                r_pos_[j] = pos;
                
                // -------------------------------------------------------------
                // step 2.3: whether this bucket width is finished scanned
                if (ldist > width && rdist > width) {
                    b_flag_[j] = false; 
                    if (++num_bucket > m_) break;
                }
                if (ldist > range && rdist > range) {
                    if (b_flag_[j]) {
                        b_flag_[j] = false; if (++num_bucket > m_) break;
                    }
                    if (r_flag_[j]) {
                        r_flag_[j] = false; if (++num_range > m_) break;
                    }
                }
            }
            if (num_bucket>m_ || num_range>m_ || cand_cnt>=cand_num) break;
        }
        // ---------------------------------------------------------------------
        //  step 3: stop condition
        if (num_range>=m_ || cand_cnt>=cand_num) break;
        
        // ---------------------------------------------------------------------
        //  step 4: update radius
        radius = c0_ * radius;
        width  = radius * w_ / 2.0f;
    }
    return cand_cnt;
}

// -----------------------------------------------------------------------------
int QALSH::dynamic_collsion_counting(// dynamic collision counting
    int   k,                            // top-k value
    std::vector<int> &cand)             // candidates (return)
{
    // k-nn search via dynamic collision counting
    int   cand_num  = CANDIDATES + k - 1; // total candidate number
    int   cand_cnt  = 0;            // candidate counter
    int   num_bucket, cnt, pos, id;
    
    float radius = 1.0f;            // search radius
    float width  = radius * w_ / 2.0f;  // bucket width
    float q_v, ldist, rdist;
    
    while (true) {
        // ---------------------------------------------------------------------
        // step 1: initialize the stop condition for current round
        num_bucket = 0; memset(b_flag_, true, sizeof(bool)*m_);
        
        // ---------------------------------------------------------------------
        // step 2: (R,c)-NN search
        while (num_bucket < m_) {
            for (int j = 0; j < m_; ++j) {
                if (!b_flag_[j]) continue;
                
                Result *table = tables_ + (u64)j*n_;
                q_v = q_val_[j], ldist = -1.0f, rdist = -1.0f;
                
                // -------------------------------------------------------------
                // step 2.1: scan the left part of hash table
                cnt = 0; pos = l_pos_[j];
                while (cnt < SCAN_SIZE) {
                    ldist = MAXREAL;
                    if (pos >= 0) ldist = fabs(q_v - table[pos].key_);
                    else break;
                    if (ldist > width) break;
                    
                    id = table[pos].id_; ++freq_[id];
                    if (freq_[id] >= l_ && !checked_[id]) {
                        checked_[id] = true;
                        cand.push_back(id);
                        if (++cand_cnt >= cand_num) break;
                    }
                    --pos; ++cnt;
                }
                if (cand_cnt >= cand_num) break;
                l_pos_[j] = pos;
                
                // -------------------------------------------------------------
                // step 2.2: scan the right part of hash table
                cnt = 0; pos = r_pos_[j];
                while (cnt < SCAN_SIZE) {
                    rdist = MAXREAL;
                    if (pos < n_) rdist = fabs(q_v - table[pos].key_);
                    else break;
                    if (rdist > width) break;
                    
                    id = table[pos].id_; ++freq_[id];
                    if (freq_[id] >= l_ && !checked_[id]) {
                        checked_[id] = true;
                        cand.push_back(id);
                        if (++cand_cnt >= cand_num) break;
                    }
                    ++pos; ++cnt;
                }
                if (cand_cnt >= cand_num) break;
                r_pos_[j] = pos;
                
                // -------------------------------------------------------------
                // step 2.3: whether this bucket width is finished scanned
                if (ldist > width && rdist > width) {
                    b_flag_[j] = false; 
                    if (++num_bucket > m_) break;
                }
            }
            if (num_bucket > m_ || cand_cnt >= cand_num) break;
        }
        // ---------------------------------------------------------------------
        //  step 3: stop condition
        if (cand_cnt >= cand_num) break;
        
        // ---------------------------------------------------------------------
        //  step 4: update radius
        radius = c0_ * radius;
        width  = radius * w_ / 2.0f;
    }
    return cand_cnt;
}

// -----------------------------------------------------------------------------
void QALSH::free()                  // free space for assistant parameters
{
    delete[] freq_;    freq_    = nullptr;
    delete[] l_pos_;   l_pos_   = nullptr;
    delete[] r_pos_;   r_pos_   = nullptr;
    delete[] checked_; checked_ = nullptr;
    delete[] b_flag_;  b_flag_  = nullptr;
    delete[] r_flag_;  r_flag_  = nullptr;
    delete[] q_val_;   q_val_   = nullptr;
}

} // end namespace ip
