#include "file_input.hpp"
#include <unordered_set>
#include <string>
#include <inttypes.h>

// -----------------------------------------------------------------------------
void output_pre_processing_results( // output pre-processing results
    const char *fname)                  // address of output file
{
    FILE *fp = fopen(fname, "a+");
    if (!fp) { printf("Could not create %s\n", fname); exit(1); }
    
    double pre_processing_time   = g_time_pre_processing / 1000000.0;
    double norm_compute_time     = g_time_norm_compute / 1000000.0;
    double lower_bound_cmpt_time = g_time_lb_computation / 1000000.0;
    double blocking_time         = g_time_blocking / 1000000.0;
    double estimated_memory      = g_memory / 1048576.0;
    
    printf("pre-processing time = %g + %g + %g = %g Seconds\n", 
        norm_compute_time, lower_bound_cmpt_time, blocking_time, 
        pre_processing_time);
    printf("estimated memory    = %.3f MB\n\n", estimated_memory);
    
    fprintf(fp, "Simpfer: k_max = %d\n", K_MAX);
    fprintf(fp, "Indexing Time: %g + %g + %g = %g Seconds\n", 
        norm_compute_time, lower_bound_cmpt_time, blocking_time, 
        pre_processing_time);
    fprintf(fp, "Estimated Memory: %f MB\n", estimated_memory);
    fclose(fp);
}

// -----------------------------------------------------------------------------
void output_reverse_kmips_results(  // output reverse kmips result for query
    int   j,                            // jth query
    std::vector<unsigned int> &result,  // reverse kmips result
    const char *fname)                  // address of output file
{
    FILE *fp = fopen(fname, j==0 ? "w" : "a+");
    if (!fp) { printf("Could not create %s\n", fname); exit(1); }
    
    if (result.size() > 0) {
        sort(result.begin(), result.end());
        fprintf(fp, "%u", result[0]);
        for (int i = 1; i < result.size(); ++i) {
            fprintf(fp, ",%u", result[i]);
        }
    }
    fprintf(fp, "\n");
    fclose(fp);
}

// -----------------------------------------------------------------------------
void output_query_results(          // output query results
    int   k,                            // top-k value
    int   qn,                           // query number
    const char *fname)                  // address of output file
{
    FILE *fp = fopen(fname, "a+");
    if (!fp) { printf("Could not create %s\n", fname); exit(1); }
    
    float avg_time  = g_time_online_processing / (1000.0 * qn);
    uint64_t avg_ip = (uint64_t) ceil((double) g_ip_count / qn);
    
    printf("top_k           = %d\n", k);
    printf("processing time = %.3f ms\n", avg_time);
    printf("#ip compuations = %"PRIu64"\n\n", avg_ip);
    
    fprintf(fp, "%d\t%f\t%"PRIu64"\n", k, avg_time, avg_ip);
    fclose(fp);
}
