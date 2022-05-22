#include "simpfer.hpp"

// -----------------------------------------------------------------------------
void create_dir(                    // create dir if the path not exists
    char *path)                         // input path
{
    int len = (int) strlen(path);
    for (int i = 0; i < len; ++i) {
        if (path[i] != '/') continue; 
        
        char ch = path[i + 1]; path[i + 1] = '\0';
        if (access(path, F_OK) != 0) { // create directory if not exists
            if (mkdir(path, 0755) != 0) {
                printf("Could not create directory %s\n", path); exit(1);
            }
        }
        path[i+1] = ch;
    }
}

// -----------------------------------------------------------------------------
int main(int nargs, char **args)
{
    srand(RANDOM_SEED);             // use a fixed random seed

    int cnt = 1;                    // parameter conunter
    unsigned int n    = 0;          // number of items
    unsigned int m    = 0;          // number of users
    unsigned int qn   = 0;          // number of query
    unsigned int d    = 0;          // dimensionality
    char items_addr[200];           // address of item  set
    char users_addr[200];           // address of user  set
    char query_addr[200];           // address of query set
    char out_folder[200];           // output folder
    
    while (cnt < nargs) {
        if (strcmp(args[cnt], "-n") == 0) {
            n = atoi(args[++cnt]); assert(n > 0);
        }
        else if (strcmp(args[cnt], "-m") == 0) {
            m = atoi(args[++cnt]); assert(m > 0);
        }
        else if (strcmp(args[cnt], "-qn") == 0) {
            qn = atoi(args[++cnt]); assert(qn > 0);
        }
        else if (strcmp(args[cnt], "-d") == 0) {
            d = atoi(args[++cnt]); assert(d > 0);
        }
        else if (strcmp(args[cnt], "-is") == 0) {
            strncpy(items_addr, args[++cnt], sizeof(items_addr));
        }
        else if (strcmp(args[cnt], "-us") == 0) {
            strncpy(users_addr, args[++cnt], sizeof(users_addr));
        }
        else if (strcmp(args[cnt], "-qs") == 0) {
            strncpy(query_addr, args[++cnt], sizeof(query_addr));
        }
        else if (strcmp(args[cnt], "-of") == 0) {
            strncpy(out_folder,  args[++cnt], sizeof(out_folder));
            create_dir(out_folder);
        }
        else {
            exit(1);
        }
        ++cnt;
    }
    std::cout << "--------------------------------------------------------\n";
    std::cout << " items number   = " << n          << "\n";
    std::cout << " users number   = " << m          << "\n";
    std::cout << " query number   = " << qn         << "\n";
    std::cout << " dimensionality = " << d          << "\n";
    std::cout << " items address  = " << items_addr << "\n";
    std::cout << " users address  = " << users_addr << "\n";
    std::cout << " query address  = " << query_addr << "\n";
    std::cout << " output folder  = " << out_folder  << "\n";
    std::cout << "--------------------------------------------------------\n\n";
    
    // -------------------------------------------------------------------------
    //  read parameters, data set & query set
    // -------------------------------------------------------------------------
    std::vector<data> item_set;     // set of item  vectors
    std::vector<data> user_set;     // set of user  vectors
    std::vector<data> query_set;    // set of query vectors
    if (read_data(n,  d, items_addr, item_set))  exit(1);
    if (read_data(m,  d, users_addr, user_set))  exit(1);
    if (read_data(qn, d, query_addr, query_set)) exit(1);
    
    // print the current time
    get_current_time();
    
    // -------------------------------------------------------------------------
    //  pre-processing
    // -------------------------------------------------------------------------
    char fname[200]; sprintf(fname, "%ssimpfer.csv", out_folder);
    std::vector<block> block_set;     // set of blocks
    
    pre_processing(d, item_set, user_set, block_set);
    output_pre_processing_results(fname);
    
    // -------------------------------------------------------------------------
    //  support online query
    // -------------------------------------------------------------------------
    std::vector<int> Ks = { 1, 5, 10, 20, 30, 40, 50 };
    std::vector<unsigned int> result;
    for (int k : Ks) {
        char k_fname[200]; 
        sprintf(k_fname, "%ssimpfer_k=%d.csv", out_folder, k);
        
        cnt = 0; g_ip_count = 0; // init global ip counter
        for (auto& query : query_set) {
            // reverse mips
            online_processing(k, d, query, item_set, block_set, result);
            
            // output result for each query
            output_reverse_kmips_results(cnt++, result, k_fname);
            
            // init parameters of user_set for next query
            for (unsigned int j = 0; j < m; ++j) user_set[j].init();
            result.clear();
        }
        output_query_results(k, qn, fname); // output query results
    }
    // print the current time
    get_current_time();
    
    return 0;
}
