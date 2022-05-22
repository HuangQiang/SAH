#include "simpfer_index.hpp"

// -----------------------------------------------------------------------------
//  verification by sequential scan
// -----------------------------------------------------------------------------
int sequential_scan(
    int   k,                            // top-k value
    unsigned int dimensionality,        // data dimensionality
    const float ip,                     // inner product of user & query 
    data *user,                         // input user
    const std::vector<data> &item_set)  // set of item vectors
{
    for (unsigned int i = 0; i < item_set.size(); ++i) {
        // guarantee to be top-k (corollary 1)
        if (ip >= user->norm_*item_set[i].norm_) return 1; // Yes
        
        // top-k comp. complete (corollary 2)
        // if (user->threshold_ >= user->norm_*item_set[i].norm_) return 1; // Yes

        // update top-k
        const float cur_ip = compute_ip(dimensionality, *user, item_set[i]);
        ++g_ip_count;
        user->update_topk(cur_ip, item_set[i].identifier_, k);

        // out from top-k (corollary 3)
        if (user->threshold_ > ip) return 0; // No
    }
    return 1; // Yes
}

// -----------------------------------------------------------------------------
void online_processing(             // find reverser k-mips for input query
    int   k,                            // top-k value
    unsigned int dimensionality,        // data dimensionality
    const data &query,                  // input query
    const std::vector<data> &item_set,  // set of item vectors
    const std::vector<block> &block_set,// set of blocks
    std::vector<unsigned int> &result)  // result (return)
{
    start = std::chrono::system_clock::now();

    // norm computation
    float norm = 0;
    for (unsigned int i = 0; i < dimensionality; ++i) {
        norm += query.vec_[i] * query.vec_[i];
    }
    norm = sqrt(norm);

    // find reverse k-mips for input query
    #pragma omp parallel num_threads(THREAD_NUM)
    {
        #pragma omp for schedule(dynamic) reduction(+:g_ip_count)
        for (unsigned int i = 0; i < block_set.size(); ++i) 
        {
            // compute upper-bound in this block
            float upperbound = block_set[i].member_[0]->norm_ * norm;
            // lemma 3 (@Qiang: suggest changing '<=' to '<')
            if (upperbound <= block_set[i].lowerbound_array_[k-1]) continue; 
            
            for (unsigned int j = 0; j < block_set[i].member_.size()-1; ++j) {
                data* user = block_set[i].member_[j]; // get user vector
                
                // comparison with lower-bound
                const float ip = compute_ip(dimensionality, query, *user);
                ++g_ip_count;
                // lemma 1 (@Qiang: suggest changing '<=' to '<')
                if (ip <= user->lowerbound_array_[k-1]) continue; 
                
                // comparison with upper-bound
                upperbound = user->norm_ * item_set[k-1].norm_;
                if (upperbound > ip) { // lemma 2
                    user->update_topk(ip, 0, k); // init topk
                    
                    // sequential scan
                    if (sequential_scan(k, dimensionality, ip, user, item_set)) {
                        result.push_back(user->identifier_);
                    }
                }
                else {
                    // add user id into the result of this query
                    result.push_back(user->identifier_);
                }
            }
        }
    }
    end = std::chrono::system_clock::now();
    double process_time = (double) std::chrono::duration_cast<std::chrono::microseconds> (end-start).count();
    g_time_online_processing += process_time;
}

