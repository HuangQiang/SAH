#include "data.hpp"

// -----------------------------------------------------------------------------
int read_data(                     // read binary data from disk
    unsigned int n,                     // data number
    unsigned int d,                     // data dimensionality
    const char *fname,                  // address of data set
    std::vector<data> &data_set)        // set of data vectors
{
    FILE *fp = fopen(fname, "rb");
    if (!fp) { printf("Could not open %s\n", fname); return 1; }
    
    data record;
    for (int i = 0; i < n; ++i) {
        std::vector<float> arr(d, 0.0f);
        fread(arr.data(), sizeof(float), d, fp);
        
        record.vec_ = arr;
        data_set.push_back(record);
        ++record.identifier_;
    }
    fclose(fp);
    return 0;
}

// -----------------------------------------------------------------------------
void get_current_time()             // get current time
{
    time_t t = time(NULL);
    printf("%s\n", ctime(&t));
}
