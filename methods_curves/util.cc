#include "util.h"

namespace ip {

timeval g_start_time;               // global param: start time
timeval g_end_time;                 // global param: end time

double g_pre_time  = 0.0;           // global param: pre-processing time (ms)
u64    g_memory    = 0;             // global param: memory usage (bytes)

u64    g_ip_count  = 0;             // global param: # ip computation counter
int    g_nq_count  = 0;             // global param: # non-empty query counter
int    g_nq_found  = 0;             // global param: # non-empty query found
double g_run_time  = 0.0;           // global param: running time (ms)
double g_recall    = 0.0;           // global param: recall (%)
double g_precision = 0.0;           // global param: precision (%)
double g_f1score   = 0.0;           // global param: f1-score (%)

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
    
    // for (int i = 0; i < (u64) n*d; ++i) data[i] *= 10000.0f;
    return 0;
}

// -----------------------------------------------------------------------------
void get_csv_from_line(             // get an array with csv format from a line
    std::string str_data,               // a string line
    std::vector<int> &csv_data)         // csv data (return)
{
    csv_data.clear();

    std::istringstream ss(str_data);
    while (ss) {
        std::string s;
        if (!getline(ss, s, ',')) break;
        csv_data.push_back(stoi(s));
    }
}

// -----------------------------------------------------------------------------
int get_conf(                       // get cand list from configuration file
    const char *data_name,              // name of dataset
    const char *method_name,            // name of method
    std::vector<int> &cand)             // candidates list (return)
{
    std::ifstream infile("config");
    if (!infile) { printf("Could not open config\n"); return 1; }

    std::string dname, mname, tmp;
    bool stop = false;
    while (infile) {
        getline(infile, dname);
        
        // check the remaining methods
        while (true) {
            getline(infile, mname);
            if (mname.length() == 0) break;

            if ((dname.compare(data_name)==0) && (mname.compare(method_name)==0)) {
                getline(infile, tmp); get_csv_from_line(tmp, cand);
                stop = true; break;
            } else {
                getline(infile, tmp);
            }
        }
        if (stop) break;
    }
    infile.close();
    return 0;
}

// -----------------------------------------------------------------------------
int read_ground_truth(              // read ground truth results from disk
    int   k,                            // top-k value
    int   qn,                           // number of query objects
    const char *truth_addr,             // address of truth set
    std::vector<std::vector<int> > &truth) // ground truth results (return)
{
    char fname[200]; sprintf(fname, "%s_k=%d.csv", truth_addr, k);
    std::ifstream infile(fname);
    if (!infile) { printf("Could not open %s\n", fname); return 1; }

    std::string tmp;
    std::vector<int> csv_data;
    for (int i = 0; i < qn; ++i) {
        getline(infile, tmp);
        get_csv_from_line(tmp, csv_data);
        truth.push_back(csv_data);
    }
    infile.close();
    return 0;
}

// -----------------------------------------------------------------------------
int write_ground_truth(             // write ground truth results to disk
    int   k,                            // top-k value
    int   qn,                           // query cardinality
    const char *truth_addr,             // address of truth set
    std::vector<std::vector<int> > &truth) // truth result (allow modify)
{
    char fname[200]; sprintf(fname, "%s_k=%d.csv", truth_addr, k);
    FILE *fp = fopen(fname, "w");
    if (!fp) { printf("Could not create %s\n", fname); return 1; }
    
    for (auto& result : truth) {
        if (result.size() > 0) {
            sort(result.begin(), result.end());

            fprintf(fp, "%d", result[0]);
            for (int i = 1; i < result.size(); ++i) {
                fprintf(fp, ",%d", result[i]);
            }
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    
    double avg_time = g_run_time * 1000.0 / qn;
    u64    avg_ip   = (u64) ceil((double) g_ip_count / qn);
    printf("k = %3d, time = %lf ms, # IPs = %lu\n", k, avg_time, avg_ip);
    if (k == K_MAX) printf("\n");
    
    return 0;
}

// -----------------------------------------------------------------------------
void output_reverse_kmips_results(  // output reverse kmips result for query
    int   i,                            // ith query
    const std::vector<int> &result,     // reverse kmips result
    const char *fname)                  // address of output file
{
    FILE *fp = fopen(fname, i==0 ? "w" : "a+");
    if (!fp) { printf("Could not create %s\n", fname); exit(1); }
    
    if (result.size() > 0) {
        fprintf(fp, "%d", result[0]);
        for (int i = 1; i < result.size(); ++i) {
            fprintf(fp, ",%d", result[i]);
        }
    }
    fprintf(fp, "\n");
    fclose(fp);
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
    printf("%s for Reverse k-Maximum Inner Product Search:\n", method_name); 
    printf("Top-k\t\tTime (ms)\t# IP Comput.\t# Non-Empty Qs\tMiss Rate (%%)\t"
        "Precision (%%)\tRecall (%%)\tF1-score (%%)\t# Candidates\n");
}

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    float b,                            // interval ratio for blocking items
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
    float b,                            // interval ratio for blocking items
    const char *method_name,            // method name
    FILE  *fp)                          // file pointer (return)
{
    fprintf(fp, "seed=%d, K=%d, b=%g\n", RANDOM_SEED, K, b);
    printf("seed=%d, K=%d, b=%g\n", RANDOM_SEED, K, b);
    head(method_name);
}

// -----------------------------------------------------------------------------
void write_params_leaf(             // write parameters
    int   leaf,                         // leaf size of Cone-Tree
    float b,                            // interval ratio for blocking items
    const char *method_name,            // method name
    FILE  *fp)                          // file pointer (return)
{
    fprintf(fp, "seed=%d, leaf=%d, b=%g\n", RANDOM_SEED, leaf, b);
    printf("seed=%d, leaf=%d, b=%g\n", RANDOM_SEED, leaf, b);
    head(method_name);
}

// -----------------------------------------------------------------------------
void write_params(                  // write parameters
    int   K,                            // # hash tables for SRP-LSH
    int   leaf,                         // leaf size of Cone-Tree
    float b,                            // interval ratio for blocking items
    const char *method_name,            // method name
    FILE  *fp)                          // file pointer (return)
{
    fprintf(fp, "seed=%d, K=%d, leaf=%d, b=%g\n", RANDOM_SEED, K, leaf, b);
    printf("seed=%d, K=%d, leaf=%d, b=%g\n", RANDOM_SEED, K, leaf, b);
    head(method_name);
}

// -----------------------------------------------------------------------------
void foot(                          // close file
    FILE *fp)                           // file pointer (return)
{
    printf("\n");
    fprintf(fp,"\n");
}

// -----------------------------------------------------------------------------
void init_global_metric()           // init the global metric
{
    g_ip_count  = 0;
    g_nq_count  = 0;
    g_nq_found  = 0;
    
    g_run_time  = 0.0;
    g_precision = 0.0;
    g_recall    = 0.0;
    g_f1score   = 0.0;
}

// -----------------------------------------------------------------------------
void update_global_metric(          // init the global metric
    const std::vector<int> &truth,      // ground truth result
    std::vector<int> &result)           // result from a method (return)
{
    u32 n = truth.size();
    if (n > 0) {
        // the ground truth result is not empty
        ++g_nq_count;
        
        u32 m = result.size();
        if (m > 0) {
            // sort results in ascending order
            sort(result.begin(), result.end());
            
            // find overlap
            u32 overlap = 0, i = 0, j = 0;  // i for truth, j for result
            while (i < n && j < m) {
                if (truth[i] < result[j]) ++i;
                else if (result[j] < truth[i]) ++j;
                else { ++overlap; ++i; ++j; }
            }
            
            // overlap is not empty => we found some truth results
            if (overlap > 0) {
                double precision = (double) overlap / (double) m;
                double recall    = (double) overlap / (double) n;
                double f1score   = 2.0*precision*recall / (precision+recall);
                
                g_precision += precision;
                g_recall    += recall;
                g_f1score   += f1score;
                ++g_nq_found;
            }
        }
    }
}

// -----------------------------------------------------------------------------
void calc_and_write_global_metric(  // init the global metric
    int  top_k,                         // top-k value
    int  cand,                          // # candidates
    int  qn,                            // number of queries
    FILE *fp)                           // file pointer
{
    double avg_time = g_run_time * 1000.0 / qn;
    u64    avg_ip   = (u64) ceil((double) g_ip_count / qn);
    
    double miss_rate = 0.0;
    double avg_pre   = 0.0;
    double avg_rec   = 0.0;
    double avg_f1    = 0.0;
    
    if (g_nq_count > 0) {
        miss_rate = (g_nq_count - g_nq_found) * 100.0 / g_nq_count;
        if (g_nq_found > 0) {
            avg_pre   = g_precision * 100.0 / g_nq_found;
            avg_rec   = g_recall    * 100.0 / g_nq_found;
            avg_f1    = g_f1score   * 100.0 / g_nq_found;
        }
    }
    else {
        miss_rate = 0.0;
        avg_pre   = 100.0;
        avg_rec   = 100.0;
        avg_f1    = 100.0;
    }
    
    printf("%3d\t\t%.3f\t\t%lu\t\t%d (%d)\t\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\t\t%d\n", 
        top_k, avg_time, avg_ip, g_nq_count, g_nq_found, miss_rate, 
        avg_pre, avg_rec, avg_f1, cand);
    fprintf(fp, "%d\t%lf\t%lu\t%d (%d)\t%lf\t%lf\t%lf\t%lf\t%d\n", 
        top_k, avg_time, avg_ip, g_nq_count, g_nq_found, miss_rate, 
        avg_pre, avg_rec, avg_f1, cand);
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
void linear_scan(                   // linear scan user_set for k-mips
    int   m,                            // number of user vectors
    int   k,                            // top-k value
    int   d,                            // dimensionality
    const float *query,                 // query vector
    const float *user_set,              // user vectors
    std::vector<int> &result)           // top-k results (return)
{
    gettimeofday(&g_start_time, nullptr);
    std::vector<int>().swap(result);// clear space for result
    
    // find top-k mips results from user_set
    MaxK_List *list = new MaxK_List(k);
    for (int j = 0; j < m; ++j) {
        const float *user = user_set + (u64) j*d;
        float ip = calc_inner_product(d, user, query);
        ++g_ip_count;
        
        list->insert(ip, j);
    }
    // copy the user ids to results
    result.resize(k);
    for (int i = 0; i < k; ++i) result[i] = list->ith_id(i);
    
    delete list;
    gettimeofday(&g_end_time, nullptr);
    
    double query_time = g_end_time.tv_sec - g_start_time.tv_sec + 
        (g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0;
    g_run_time += query_time;
}

} // end namespace ip
