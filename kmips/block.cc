#include "block.h"

namespace ip {

// -----------------------------------------------------------------------------
Block::Block(                       // constructor
    int   n,                            // number of data
    float M,                            // max l2-norm of data
    const int   *index,                 // data index
    const float *norms,                 // data l2-norms
    const float *data)                  // data points
    : n_(n), M_(M), R_(-1.0f), index_(index), norms_(norms), data_(data), 
    lsh_(nullptr), srp_(nullptr)
{
}

// -----------------------------------------------------------------------------
Block::~Block()                     // destructor
{
    if (lsh_ != nullptr) { delete lsh_; lsh_ = nullptr; }
    if (srp_ != nullptr) { delete srp_; srp_ = nullptr; }
}

} // end namespace ip
