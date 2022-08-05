#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <cstring>
#include <functional>
#include <map>
#include <random>
#include <string>
#include <vector>

#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

// -----------------------------------------------------------------------------
//  public parameters
// -----------------------------------------------------------------------------
const unsigned int K_MAX = 50;      // k_max
const float COEFF = 4.0f;           // constant for O(k_max)

const unsigned int THREAD_NUM = 1;  // # threads
const unsigned int RANDOM_SEED = 6; // random seed
const float SAMPLING_RATE = 1.0f;   // dataset sampling rate

uint64_t g_ip_count = 0UL;          // ip computation counter
uint64_t g_memory = 0UL;            // memory usage (bytes)
double g_time_online_processing = 0;// total online computation time
double g_time_pre_processing = 0;   // total pre-processing time
double g_time_norm_compute   = 0;   // norm computation time
double g_time_lb_computation = 0;   // lower bound computation time
double g_time_blocking       = 0;   // blocking time

std::chrono::system_clock::time_point start, end; // computation time clock

// -----------------------------------------------------------------------------
//  definition of data
// -----------------------------------------------------------------------------
class data {
public:
    unsigned int identifier_ ;      // data  id, start from 0
    unsigned int block_id_;         // block id, start from 0
    
    float norm_;                    // l_2 norm of data
    float threshold_ ;              // k-th largest ip value
    std::vector<float> vec_;        // data
    std::map<float, unsigned int, std::greater<float> > topk_; //top-k pairs
    std::vector<float> lowerbound_array_; // lower bound array
    
    // -------------------------------------------------------------------------
    data()                          // constructor
    : identifier_(0), block_id_(0), norm_(0.0f), threshold_(0.0f) {}
    
    // -------------------------------------------------------------------------
    void init() {                   // init
        topk_.clear();  // clear top-k in pre-processing
        threshold_ = 0; // clear threshold in pre-processing
    }
    
    // -------------------------------------------------------------------------
    void norm_computation() {       // norm computation
        norm_ = 0.0f;
        for (unsigned int i = 0; i < vec_.size(); ++i) {
            norm_ += vec_[i] * vec_[i];
        }
        norm_ = sqrt(norm_);
    }
    
    // -------------------------------------------------------------------------
    void update_topk(               // k-mips update
        const float ip,                 // inner product value
        const unsigned int id,          // input id
        const unsigned int k)           // top-k value
    {
        if (ip > threshold_) topk_.insert({ip,id});
        
        if (topk_.size() > k) {
            auto it = topk_.end();
            --it;                   // get the last element
            topk_.erase(it);        // delete the last element
        }

        if (topk_.size() == k) {
            auto it = topk_.end();
            --it;                   // get the last element
            threshold_ = it->first; // update threshold_ by the k-th value
        }
    }
    
    // -------------------------------------------------------------------------
    void make_lb_array() {          // init lower-bound array
        auto it = topk_.begin();
        while(it != topk_.end()) {
            lowerbound_array_.push_back(it->first);
            ++it;
        }
    }
    
    // -------------------------------------------------------------------------
    uint64_t get_estimated_memory() { // get estimated memory (except data)
        uint64_t ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(float)*lowerbound_array_.size(); // lowerbound_array_
        
        return ret;
    }
    
    bool operator <(const data &d) const { return norm_ < d.norm_; }
    bool operator >(const data &d) const { return norm_ > d.norm_; }
};

// -----------------------------------------------------------------------------
//  definition of block
// -----------------------------------------------------------------------------
class block{
public:
    unsigned int identifier_;       // block id
    std::vector<data*> member_;     // user i
    std::vector<float> lowerbound_array_; // lower bound array for block
    
    // -------------------------------------------------------------------------
    block() : identifier_(0) {      // constructor
        lowerbound_array_.resize(K_MAX);
        for (unsigned int i = 0; i < K_MAX; ++i) {
            lowerbound_array_[i] = FLT_MAX;
        }
    }
    
    // -------------------------------------------------------------------------
    void init() {                   // init
        ++identifier_;              // increment identifier
        member_.clear();            // clear member
        for (unsigned int i = 0; i < K_MAX; ++i) { // init array
            lowerbound_array_[i] = FLT_MAX;
        }
    }
    
    // -------------------------------------------------------------------------
    void update_lowerbound_array() {// make lower-bound array
        for (unsigned int i = 0; i < K_MAX; ++i) {
            for (unsigned int j = 0; j < member_.size(); ++j) {
                if (lowerbound_array_[i] > member_[j]->lowerbound_array_[i]) {
                    lowerbound_array_[i] = member_[j]->lowerbound_array_[i];
                }
            }
        }
    }
    
    // -------------------------------------------------------------------------
    uint64_t get_estimated_memory() { // get estimated memory (except data)
        uint64_t ret = 0UL;
        ret += sizeof(*this);
        ret += sizeof(data*)*member_.size(); // member_
        ret += sizeof(float)*lowerbound_array_.size(); // lowerbound_array_
        
        return ret;
    }
};

