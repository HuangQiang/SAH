#include "block.h"

namespace ip {

// -----------------------------------------------------------------------------
Item_Block::Item_Block(             // constructor
    int   n,                            // number of items
    float M,                            // max l2-norm of items
    const float *norms,                 // l2-norms of items
    const float *items)                 // items
    : n_(n), M_(M), R_(-1.0f), norms_(norms), items_(items), lsh_(nullptr),
    srp_(nullptr)
{
}

// -----------------------------------------------------------------------------
Item_Block::~Item_Block()           // destructor
{
    if (lsh_ != nullptr) { delete lsh_; lsh_ = nullptr; }
    if (srp_ != nullptr) { delete srp_; srp_ = nullptr; }
}


// -----------------------------------------------------------------------------
User_Block::User_Block(             // constructor
    int   m,                            // number of users
    int   k_max,                        // max k value
    const int   *index,                 // index of users
    const float *norms,                 // l2-norms of users
    const float *users,                 // users
    const float *lower_bounds)          // lower bounds of users
    : m_(m), k_max_(k_max), index_(index), norms_(norms), users_(users),
    lower_bounds_(lower_bounds)
{
    // init block lower bounds
    block_lower_bounds_ = new float[k_max];
    for (int i = 0; i < k_max; ++i) block_lower_bounds_[i] = MAXREAL;
    
    // update block lower bounds
    for (int i = 0; i < m; ++i) {
        const float *lb = lower_bounds + (u64) i*k_max;
        for (int j = 0; j < k_max; ++j) {
            if (block_lower_bounds_[j] > lb[j]) block_lower_bounds_[j] = lb[j];
        }
    }
}

// -----------------------------------------------------------------------------
User_Block::~User_Block()           // destructor
{
    if (block_lower_bounds_ != nullptr) {
        delete[] block_lower_bounds_; block_lower_bounds_ = nullptr;
    }
}

} // end namespace ip
