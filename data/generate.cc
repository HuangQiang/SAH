#include <cstdint>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <cstring>
#include <vector>

#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

typedef uint64_t u64;

// -----------------------------------------------------------------------------
int read_bin_data(                  // read bin data from disk
    int   n,                            // number of data points
    int   d,                            // data dimension
    const char *fname,                  // address of data set
    float *data)                        // data (return)
{
    // read bin data
    FILE *fp = fopen(fname, "rb");
    if (!fp) { printf("Could not open %s\n", fname); return 1; }
    
    fread(data, sizeof(float), (u64) n*d, fp);
    fclose(fp);
    
    return 0;
}

// -----------------------------------------------------------------------------
int write_bin_data(                 // write binary data to disk
    int   n,                            // number of data points
    int   d,                            // data dimension
    char  *fname,                       // output file name
    const float *data)                  // output data
{
    //  write binary data
    FILE *fp = fopen(fname, "wb");
    if (!fp) { printf("Could not open %s\n", fname); return 1; }
    
    fwrite(data, sizeof(float), (u64) n*d, fp);
    fclose(fp);
    
    return 0;
}

// -----------------------------------------------------------------------------
void generate_query(                // generate query
    int   n,                            // number of data points
    int   qn,                           // number of hyperplane queries
    int   d,                            // data dimension + 1
    const float *data,                  // input data
    float *query)                       // query (return)
{
    // init an array
    int *arr = new int[n];
    for (int i = 0; i < n; ++i) arr[i] = i;
    
    // random shuffle
    std::random_shuffle(arr, arr+n);
    
    // extract the first qn data as queries
    for (int i = 0; i < qn; ++i) {
        printf("id = %d\n", arr[i]);
        const float *one_data = data + (u64)arr[i]*d;
        float *one_query = query + (u64)i*d;
        
        std::copy(one_data, one_data+d, one_query);
    }
    
    // for (int i = 0; i < qn; ++i) {
    //     for (int j = 0; j < d; ++j) printf("%g ", query[i*d+j]);
    //     printf("\n");
    // }
    // printf("\n");
}

// -----------------------------------------------------------------------------
int main(int nargs, char** args)
{
    srand(666); // set up random seed 

    // read parameters
    int  n  = atoi(args[1]); // cardinality
    int  d  = atoi(args[2]); // dimensionality
    int  qn = atoi(args[3]); // number of queries
    char ifile[200]; strncpy(ifile, args[4], sizeof(ifile));
    char ofile[200]; strncpy(ofile, args[5], sizeof(ofile));

    printf("n           = %d\n", n);
    printf("d           = %d\n", d);
    printf("qn          = %d\n", qn);
    printf("input  file = %s\n", ifile);
    printf("output file = %s\n", ofile);
    printf("\n");

    // read dataset
    float *data = new float[(u64) n*d];
    read_bin_data(n, d, ifile, data);
    
    // generate query
    float *query = new float[(u64) qn*d];
    generate_query(n, qn, d, data, query);

    // write query to disk
    write_bin_data(qn, d, ofile, query);
    
    // release space
    delete[] data;
    delete[] query;

    return 0;
}
