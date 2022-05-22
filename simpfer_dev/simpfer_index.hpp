#include "file_output.hpp"

// -----------------------------------------------------------------------------
float compute_ip(                   // ip computation
    unsigned int dimensionality,        // data dimensionality
    const data &q,                      // vector q
    const data &p)                      // vector p
{
    float ip = 0;
    for(unsigned int i = 0; i < dimensionality; ++i) {
        ip += q.vec_[i] * p.vec_[i];
    }
    return ip;
}

// -----------------------------------------------------------------------------
void compute_norm(                  // norm computation
    std::vector<data> &item_set,        // set of item vectors (return)
    std::vector<data> &user_set)        // set of user vectors (return)
{
    start = std::chrono::system_clock::now();
    
    // norm computation
    for (unsigned int i = 0; i < user_set.size(); ++i) {
        user_set[i].norm_computation();
    }
    for (unsigned int i = 0; i < item_set.size(); ++i) {
        item_set[i].norm_computation();
    }
    
    // sort by norm in descending order
    std::sort(user_set.begin(), user_set.end(), std::greater<data>());
    std::sort(item_set.begin(), item_set.end(), std::greater<data>());

    end = std::chrono::system_clock::now();
    g_time_norm_compute = (double) std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();
    g_time_pre_processing += g_time_norm_compute;
    printf("Norm computation time: %lf Seconds\n", g_time_norm_compute/1000000.0);
}

// -----------------------------------------------------------------------------
void compute_lowerbound(            // lower-bound computation
    unsigned int dimensionality,        // data dimensionality
    const std::vector<data> &item_set,  // set of item vectors
    std::vector<data> &user_set)        // set of user vectors (return)
{
    start = std::chrono::system_clock::now();
    
    unsigned int k_size = K_MAX * COEFF;
    for (unsigned int i = 0; i < user_set.size(); ++i) {
        // only consider the first k_size items for lower bound computation
        for (unsigned int j = 0; j < k_size; ++j) {
            // ip computation
            const float ip = compute_ip(dimensionality, user_set[i], item_set[j]);
            
            // update top-k
            user_set[i].update_topk(ip, item_set[j].identifier_, K_MAX);
        }
        // convert map to array
        user_set[i].make_lb_array();
    }
    end = std::chrono::system_clock::now();
    g_time_lb_computation = (double) std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();
    g_time_pre_processing += g_time_lb_computation;
    printf("Lower-bound computation time: %lf Seconds\n", g_time_lb_computation/1000000.0);
}

// -----------------------------------------------------------------------------
void blocking(                      // blocking
    std::vector<data>  &user_set,       // set of user vectors (return)
    std::vector<block> &block_set)      // set of blocks
{
    start = std::chrono::system_clock::now();
    
    // TODO determine size (use user_set instead of item_set)
    const unsigned int block_size = (unsigned int) (log2(user_set.size())*2);

    // make block
    block blk;
    for (unsigned int i = 0; i < user_set.size(); ++i) {
        // assign block id
        user_set[i].block_id_ = blk.identifier_;

        // insert into block
        blk.member_.push_back(&user_set[i]);

        // init blk
        if (blk.member_.size() == block_size) {
            blk.update_lowerbound_array(); // update lower-bound array
            block_set.push_back(blk);      // insert into set
            
            blk.init(); // init
        }
    }
    end = std::chrono::system_clock::now();
    g_time_blocking = (double) std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();
    g_time_pre_processing += g_time_blocking;
    printf("Blocking time: %lf Seconds\n\n", g_time_blocking/1000000.0);
}

// -----------------------------------------------------------------------------
void pre_processing(                // pre-process item & user sets & build block
    unsigned int dimensionality,        // data dimensionality
    std::vector<data>  &item_set,       // set of item vectors (return)
    std::vector<data>  &user_set,       // set of user vectors (return)
    std::vector<block> &block_set)      // set of blocks (return)
{
    // norm computation for item_set and user_set and sorting
    compute_norm(item_set, user_set);

    // lower-bound computation for user_set
    compute_lowerbound(dimensionality, item_set, user_set);

    // build blocks for user_set
    blocking(user_set, block_set);

    // init parameters of user_set for online query
    for (unsigned int i = 0; i < user_set.size(); ++i) user_set[i].init();
    
    // get memory size
    g_memory = 0UL;
    for (auto& item : item_set)  g_memory += item.get_estimated_memory();
    for (auto& user : user_set)  g_memory += user.get_estimated_memory();
    for (auto& block: block_set) g_memory += block.get_estimated_memory();
}
