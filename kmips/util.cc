#include "util.h"

namespace ip {

timeval g_start_time;               // global param: start time
timeval g_end_time;                 // global param: end time

double g_pre_time = 0.0;            // global param: pre-processing time (ms)
u64    g_memory   = 0;              // global param: memory usage (bytes)
u64    g_cand_cnt = 0;              // global param: # candidates counter
double g_run_time = 0.0;            // global param: running time (ms)
double g_recall   = 0.0;            // global param: recall (%)
double g_ratio    = 0.0;            // global param: overall ratio

// -----------------------------------------------------------------------------
//  Input & Output
// -----------------------------------------------------------------------------
void create_dir(                    // create dir if the path not exists
    char *path)                         // input path
{
    int len = (int) strlen(path);
    for (int i = 0; i < len; ++i) {
        if (path[i] != '/') continue; 
        
        char ch = path[i+1]; path[i+1] = '\0';
        if (access(path, F_OK) != 0) { // create directory if not exists
            if (mkdir(path, 0755) != 0) {
                printf("Could not create directory %s\n", path); exit(1);
            }
        }
        path[i+1] = ch;
    }
}

// -----------------------------------------------------------------------------
int read_bin_data(                  // read binary data from disk
    int   n,                            // number of data
    int   d,                            // dimensionality
    const char *fname,                  // address of data
    float *data)                        // data (return)
{
    FILE *fp = fopen(fname, "rb");
    if (!fp) { printf("Could not open %s\n", fname); return 1; }
    
    fread(data, sizeof(float), (u64) n*d, fp);
    fclose(fp);
    
    return 0;
}

// -----------------------------------------------------------------------------
int read_ground_truth(              // read ground truth results from disk
    int    qn,                          // number of query points
    int    k_max,                       // k max value
    const  char *truth_addr,            // address of truth set
    Result *truth_set)                  // ground truth results (return)
{
    FILE *fp = fopen(truth_addr, "r");
    if (!fp) { printf("Could not open %s\n", truth_addr); return 1; }
    
    int t1 = -1, t2 = -1;
    fscanf(fp, "%d %d\n", &t1, &t2);
    assert(t1 == qn && t2 == k_max);
    
    for (int i = 0; i < qn; ++i) {
        Result *truth = truth_set + i*k_max;
        for (int j = 0; j < k_max; ++j) {
            fscanf(fp, "%d %f ", &truth[j].id_, &truth[j].key_);
        }
        fscanf(fp, "\n");
    }
    fclose(fp);
    return 0;
}

// -----------------------------------------------------------------------------
int write_ground_truth(             // write ground truth results to disk
    int   qn,                           // number of query points
    int   k_max,                        // k max value
    const char *truth_addr,             // address of truth set
    const Result *truth_set)            // ground truth results
{
    FILE *fp = fopen(truth_addr, "w");
    if (!fp) { printf("Could not create %s\n", truth_addr); return 1; }
    
    fprintf(fp, "%d %d\n", qn, k_max);
    for (int i = 0; i < qn; ++i) {
        const Result *truth = truth_set + i*k_max;
        for (int j = 0; j < k_max; ++j) {
            fprintf(fp, "%d %f ", truth[j].id_, truth[j].key_);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    return 0;
}

// -----------------------------------------------------------------------------
void write_index_info(              // display & write index overhead info
    FILE *fp)                           // file pointer (return)
{
    double memory = g_memory / 1048576.0;// convert bytes into megabytes
    
    printf("Indexing Time: %g Seconds\n", g_pre_time);
    printf("Estimated Mem: %g MB\n\n", memory);

    fprintf(fp, "Indexing Time: %g Seconds\n", g_pre_time);
    fprintf(fp, "Estimated Memory: %g MB\n", memory);
}

// -----------------------------------------------------------------------------
void head(                          // display head with method name
    const char *method_name)            // method name
{
    printf("%s for k-Maximum Inner Product Search:\n", method_name); 
    printf("Top-k\tTime (ms)\t# Candidates\tRatio\t\tRecall (%%)\n");
}

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    const char *method_name,            // method name
    FILE  *fp)                          // file pointer (return)
{
    fprintf(fp, "seed=%d\n", RANDOM_SEED);
    printf("seed=%d\n", RANDOM_SEED);
    head(method_name);
}

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    float b,                            // interval ratio
    const char *method_name,            // method name
    FILE  *fp)                          // file pointer (return)
{
    fprintf(fp, "seed=%d, b=%g\n", RANDOM_SEED, b);
    printf("seed=%d, b=%g\n", RANDOM_SEED, b);
    head(method_name);
}

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    int   K,                            // # hash tables for SRP-LSH
    float b,                            // interval ratio
    const char *method_name,            // method name
    FILE  *fp)                          // file pointer (return)
{
    fprintf(fp, "seed=%d, K=%d, b=%g\n", RANDOM_SEED, K, b);
    printf("seed=%d, K=%d, b=%g\n", RANDOM_SEED, K, b);
    head(method_name);
}

// -----------------------------------------------------------------------------
void init_global_metric()           // init the global metric
{
    g_cand_cnt = 0;
    g_run_time = 0.0;
    g_recall   = 0.0;
    g_ratio    = 0.0;
}

// -----------------------------------------------------------------------------
float calc_ratio(                   // calc overall ratio
    int   k,                            // top-k value
    const Result *R,                    // ground truth results 
    MaxK_List *list)                    // results returned by algorithms
{
    // add penalty if list->size() < k
    if (list->size() < k) return MAXREAL;

    // consider geometric mean instead of arithmetic mean for overall ratio
    float sum = 0.0f, r = -1.0f;
    for (int j = 0; j < k; ++j) {
        if (fabs(list->ith_key(j) - R[j].key_) < CHECK_ERROR) r = 1.0f;
        else r = (R[j].key_ + 1e-9) / (list->ith_key(j) + 1e-9);
        sum += log(r);
    }
    return pow(E, sum / k);
}

// -----------------------------------------------------------------------------
float calc_recall(                  // calc recall of mip results
    int   k,                            // top-k value
    const Result *R,                    // ground truth results 
    MaxK_List *list)                    // results returned by algorithms
{
    int i = list->size() - 1;
    int last = k - 1;
    while (i >= 0 && R[last].key_ - list->ith_key(i) > CHECK_ERROR) {
        --i;
    }
    return (i + 1) * 100.0f / k;
}

// -----------------------------------------------------------------------------
void update_global_metric(          // init the global metric
    int   k,                            // top-k value
    const Result *truth,                // ground truth result
    MaxK_List *list)                    // top-k results by a method
{
    g_ratio  += calc_ratio(k,  truth, list);
    g_recall += calc_recall(k, truth, list);
}

// -----------------------------------------------------------------------------
void calc_and_write_global_metric(  // init the global metric
    int  k,                             // top-k value
    int  qn,                            // number of queries
    FILE *fp)                           // file pointer
{
    u64    avg_cand_cnt = (u64) ceil((double) g_cand_cnt / qn);
    double avg_run_time = g_run_time * 1000.0 / qn;
    double avg_ratio    = g_ratio / qn;
    double avg_recall   = g_recall / qn;
    
    printf("%d\t%.3f\t\t%lu\t\t%.4f\t\t%.2f%%\n", k, avg_run_time, 
        avg_cand_cnt, avg_ratio, avg_recall);
    fprintf(fp, "%d\t%f\t%lu\t%f\t%f\n", k, avg_run_time, avg_cand_cnt, 
        avg_ratio, avg_recall);
}

// -----------------------------------------------------------------------------
void foot(                          // close file
    FILE *fp)                           // file pointer (return)
{
    printf("\n");
    fprintf(fp,"\n");
}

// -----------------------------------------------------------------------------
//  Generate random variables
// -----------------------------------------------------------------------------
float uniform(                      // r.v. from Uniform(min, max)
    float min,                          // min value
    float max)                          // max value
{
    int   num  = rand();
    float base = (float) RAND_MAX - 1.0F;
    float frac = ((float) num) / base;

    return (max - min) * frac + min;
}

// -----------------------------------------------------------------------------
//  Given a mean and a standard deviation, gaussian generates a normally 
//  distributed random number.
//
//  Algorithm:  Polar Method, p.104, Knuth, vol. 2
// -----------------------------------------------------------------------------
float gaussian(                     // r.v. from Gaussian(mean, sigma)
    float mean,                         // mean value
    float sigma)                        // std value
{
    float v1 = -1.0f, v2 = -1.0f, s = -1.0f, x = -1.0f;
    do {
        v1 = 2.0f * uniform(0.0f, 1.0f) - 1.0f;
        v2 = 2.0f * uniform(0.0f, 1.0f) - 1.0f;
        s = v1 * v1 + v2 * v2;
    } while (s >= 1.0f);
    x = v1 * sqrt(-2.0f * log(s) / s);

    // x is distributed from N(0, 1)
    return x * sigma + mean;
}

// -----------------------------------------------------------------------------
float normal_cdf(                   // cdf of N(0, 1) in range (-inf, x]
    float _x,                           // integral border
    float _step)                        // step increment
{
    float ret = 0.0f;
    for (float i = -10.0f; i < _x; i += _step) {
        ret += _step * normal_pdf(i, 0.0f, 1.0f);
    }
    return ret;
}

// -----------------------------------------------------------------------------
float new_cdf(                      // cdf of N(0, 1) in range [-x, x]
    float x,                            // integral border
    float step)                         // step increment
{
    float result = 0.0f;
    for (float i = -x; i <= x; i += step) {
        result += step * normal_pdf(i, 0.0f, 1.0f);
    }
    return result;
}

// -----------------------------------------------------------------------------
//  Distance and similarity functions
// -----------------------------------------------------------------------------
float calc_l2_sqr(                  // calc l_2 distance square
    int   dim,                          // dimensionality
    const float *p1,                    // 1st point
    const float *p2)                    // 2nd point
{
    float ret = 0.0f;
    for (int i = 0; i < dim; ++i) ret += SQR(p1[i] - p2[i]);

    return ret;
}

// -----------------------------------------------------------------------------
float calc_l2_dist(                 // calc l_2 distance
    int   dim,                          // dimensionality
    const float *p1,                    // 1st point
    const float *p2)                    // 2nd point
{
    return sqrt(calc_l2_sqr(dim, p1, p2));
}

// -----------------------------------------------------------------------------
float calc_inner_product(           // calc inner product
    int   dim,                          // dimensionality
    const float *p1,                    // 1st point
    const float *p2)                    // 2nd point
{
    float ret = 0.0f;
    for (int i = 0; i < dim; ++i) ret += p1[i] * p2[i];
    
    return ret;
}

// -----------------------------------------------------------------------------
float calc_cosine_angle(            // calc cosine angle, [-1,1]
    int   dim,                          // dimensionality
    const float *p1,                    // 1st point
    const float *p2)                    // 2nd point
{
    float ip    = calc_inner_product(dim, p1, p2);
    float norm1 = calc_inner_product(dim, p1, p1);
    float norm2 = calc_inner_product(dim, p2, p2);

    return ip / sqrt(norm1 * norm2);
}

// -----------------------------------------------------------------------------
float calc_angle(                   // calc angle between two points
    int   dim,                          // dimension
    const float *p1,                    // 1st point
    const float *p2)                    // 2nd point
{
    // acos returns an angle in [0,pi]
    return acos(calc_cosine_angle(dim, p1, p2));
}

// -----------------------------------------------------------------------------
void calc_centroid(                 // calc centroid
    int   n,                            // number of data points
    int   dim,                          // dimensionality
    const float *data,                  // data points
    float *centroid)                    // centroid (return)
{
    memset(centroid, 0.0f, sizeof(float)*dim);
    for (int i = 0; i < n; ++i) {
        const float *point = data + (u64) i*dim;
        for (int j = 0; j < dim; ++j) centroid[j] += point[j];
    }
    for (int i = 0; i < dim; ++i) centroid[i] /= (float) n;
}

// -----------------------------------------------------------------------------
void calc_centroid(                 // calc centroid
    int   n,                            // size of data index
    int   dim,                          // dimensionality
    const int   *index,                 // data index
    const float *data,                  // data points
    float *centroid)                    // centroid (return)
{
    memset(centroid, 0.0f, sizeof(float)*dim);
    for (int i = 0; i < n; ++i) {
        const float *point = data + (u64) index[i]*dim;
        for (int j = 0; j < dim; ++j) centroid[j] += point[j];
    }
    for (int i = 0; i < dim; ++i) centroid[i] /= (float) n;
}

// -----------------------------------------------------------------------------
float shift_data_and_norms(         // calc shifted data and their l2-norm sqrs
    int   n,                            // number of data vectors
    int   d,                            // dimensionality
    const float *data,                  // data vectors
    const float *centroid,              // centroid
    float *shift_data,                  // shifted data vectors (return)
    float *shift_norms)                 // shifted l2-norm sqrs (return)
{
    float max_norm_sqr = -1.0f;
    for (int i = 0; i < n; ++i) {
        const float *record = data + (u64) i*d;
        float *shift_record = shift_data + (u64) i*d;
        
        // calc shifted data & its l2-norm
        float norm = 0.0f;
        for (int j = 0; j < d; ++j) {
            float diff = record[j] - centroid[j];
            
            shift_record[j] = diff; 
            norm += SQR(diff);
        }
        // update shift_norm & max l2-norm
        shift_norms[i] = norm;
        if (max_norm_sqr < norm) max_norm_sqr = norm;
    }
    return max_norm_sqr;
}

// -----------------------------------------------------------------------------
void kmips(                         // k-MIPS by linear scan
    int   n,                            // number of data points
    int   d,                            // dimensionality
    const float *data_set,              // data points
    const float *query,                 // query point
    MaxK_List *list)                    // top-k results (return)
{
    gettimeofday(&g_start_time, nullptr);
    
    list->reset();
    for (int j = 0; j < n; ++j) {
        const float *data = data_set + (u64) j*d;
        float ip = calc_inner_product(d, data, query);
        ++g_cand_cnt;
        
        list->insert(ip, j+1);
    }
    gettimeofday(&g_end_time, nullptr);
    
    double query_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_run_time += query_time;
}

} // end namespace ip
