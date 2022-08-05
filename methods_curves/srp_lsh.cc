#include "srp_lsh.h"

namespace ip {

// -----------------------------------------------------------------------------
SRP_LSH::SRP_LSH(                   // constructor
    int   n,                            // cardinality of dataset
    int   d,                            // dimensionality of dataset
    int   K)                            // number of hash tables
    : n_(n), d_(d), K_(K), m_(K/64)
{
    assert(K % 64 == 0);
    // m_ = (int) ceil((double) K / 64.0);
    // align_ = K%64 == 0;
    
    // generate random projection vectors
    int size = K*d;
    proj_ = new float[size];
    for (int i = 0; i < size; ++i) proj_[i] = gaussian(0.0f, 1.0f);
    
    // initialize lookup table for all uint16_t values
    size = 1 << 16;
    table16_ = new u32[size];
    for (u32 i = 0; i < size; ++i) table16_[i] = bit_count(i);
    
    // allocate space for hash_key
    hash_keys_ = new u64[n*m_];
}

// -----------------------------------------------------------------------------
bool SRP_LSH::calc_hash_code(       // calc hash code after random projection
    int   id,                           // projection vector id
    const float *data)                  // input data
{
    float *proj = proj_ + id*d_;
    return calc_inner_product(d_, proj, data) >= 0;
}

// -----------------------------------------------------------------------------
void SRP_LSH::compress_hash_code(   // compress hash code with 64 bits
    const bool *hash_code,              // hash code
    u64   *hash_key)                    // hash key (return)
{
    int size=64, shift=0;
    for (int i = 0; i < m_; ++i) {
        // if (!align_ && i == m_-1) size = K_%64;
        
        const bool *single_code = hash_code + shift;
        u64 val = 0;
        for (int j = 0; j < size; ++j) {
            if (single_code[j]) val |= (1UL << (63-j));
        }
        hash_key[i] = val;
        shift += size;
    }
}

// -----------------------------------------------------------------------------
SRP_LSH::~SRP_LSH()                 // destructor
{
    if (!proj_)      { delete[] proj_;      proj_      = nullptr; }
    if (!table16_)   { delete[] table16_;   table16_   = nullptr; }
    if (!hash_keys_) { delete[] hash_keys_; hash_keys_ = nullptr; }
}

// -----------------------------------------------------------------------------
void SRP_LSH::display()             // display parameters
{
    printf("Parameters of SRP_LSH:\n");
    printf("n     = %d\n", n_);
    printf("d     = %d\n", d_);
    printf("K     = %d\n", K_);
    printf("m     = %d\n", m_);
    // printf("align = %s\n", align_ ? "Yes" : "No");
    printf("\n");
}

// -----------------------------------------------------------------------------
int SRP_LSH::kmcss(                 // k-mcss
    int   k,                            // top-k value
    const float *query,                 // input query
    std::vector<int> &cand)             // k-mcss candidates (return)
{
    cand.clear();
    
    // calculate the hash key (compressed hash code) of query
    bool *hash_code_q = new bool[K_];
    for (int i = 0; i < K_; ++i) {
        hash_code_q[i] = calc_hash_code(i, query); ++g_ip_count;
    }
    
    u64 *hash_key_q = new u64[m_];
    compress_hash_code(hash_code_q, hash_key_q);

    // find the candidates with largest matched values
    MaxK_List *list = new MaxK_List(CANDIDATES+k-1);
    int total_bits = 64*m_;
    for (int i = 0; i < n_; ++i) {
        // get hash_key for ith data
        const u64 *hash_key = hash_keys_ + (u64) i*m_;
        
        // calc the number of matches
        u32 match = 0;
        for (int j = 0; j < m_; ++j) {
            match += table_lookup(hash_key[j] ^ hash_key_q[j]);
        }
        list->insert((float) (total_bits - match), i);
    }
    // update candidates
    int num = list->num_;
    Result *k_list = list->list_;
    cand.resize(num);
    for (int i = 0; i < num; ++i) cand[i] = k_list[i].id_;

    // release space
    delete[] hash_code_q;
    delete[] hash_key_q;
    delete list;

    return 0;
}

} // end namespace ip
