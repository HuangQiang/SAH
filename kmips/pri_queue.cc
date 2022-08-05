#include "pri_queue.h"

namespace ip {

// -----------------------------------------------------------------------------
int ResultComp(                     // compare function for qsort (ascending)
    const void *e1,                     // 1st element
    const void *e2)                     // 2nd element
{
    int ret = 0;
    Result *item1 = (Result*) e1;
    Result *item2 = (Result*) e2;

    if (item1->key_ < item2->key_) {
        ret = -1;
    } 
    else if (item1->key_ > item2->key_) {
        ret = 1;
    } 
    else {
        if (item1->id_ < item2->id_) ret = -1;
        else if (item1->id_ > item2->id_) ret = 1;
    }
    return ret;
}

// -----------------------------------------------------------------------------
int ResultCompDesc(                 // compare function for qsort (descending)
    const void *e1,                     // 1st element
    const void *e2)                     // 2nd element
{
    int ret = 0;
    Result *item1 = (Result*) e1;
    Result *item2 = (Result*) e2;

    if (item1->key_ < item2->key_) {
        ret = 1;
    } 
    else if (item1->key_ > item2->key_) {
        ret = -1;
    } 
    else {
        if (item1->id_ < item2->id_) ret = -1;
        else if (item1->id_ > item2->id_) ret = 1;
    }
    return ret;
}


// -----------------------------------------------------------------------------
MinK_List::MinK_List(               // constructor (given k size)
    int k)                              // k size
    : num_(0), k_(k)
{
    list_ = new Result[k+1];
}

// -----------------------------------------------------------------------------
MinK_List::~MinK_List()             // destructor
{
    if (list_ != nullptr) { delete[] list_; list_ = nullptr; }
}

// -----------------------------------------------------------------------------
float MinK_List::insert(            // insert item (inline for speed)
    float key,                          // key of item
    int   id)                           // id of item
{
    int i = 0;
    for (i = num_; i > 0; --i) {
        if (key < list_[i-1].key_) list_[i] = list_[i-1];
        else break;
    }
    list_[i].key_ = key;    // store new item
    list_[i].id_  = id;
    if (num_ < k_) ++num_;  // increase the number of items

    return max_key();
}


// -----------------------------------------------------------------------------
MaxK_List::MaxK_List(               // constructor (given k size)
    int k)                              // k size
    : num_(0), k_(k)
{
    list_ = new Result[k+1];
}

// -----------------------------------------------------------------------------
MaxK_List::~MaxK_List()             // destructor
{
    if (list_ != nullptr) { delete[] list_; list_ = nullptr; }
}

// -----------------------------------------------------------------------------
float MaxK_List::insert(            // insert item
    float key,                          // key of item
    int   id)                           // id of item
{
    int i = 0;
    for (i = num_; i > 0; --i) {
        if (list_[i-1].key_ < key) list_[i] = list_[i-1];
        else break;
    }
    list_[i].key_ = key;    // store new item
    list_[i].id_  = id;
    if (num_ < k_) ++num_;  // increase the number of items

    return min_key();
}


// -----------------------------------------------------------------------------
MaxK_Array::MaxK_Array(             // constructor (given k size)
    int k)                              // k size
    : num_(0), k_(k)
{
    keys_ = new float[k+1];
}

// -----------------------------------------------------------------------------
MaxK_Array::~MaxK_Array()           // destructor
{
    if (keys_ != nullptr) { delete[] keys_; keys_ = nullptr; }
}

// -----------------------------------------------------------------------------
void MaxK_Array::init(              // init with an sorted array
    int   k,                            // number of keys
    const float *keys)                  // input keys
{
    num_ = std::min(k_, k);
    std::copy(keys, keys + num_, keys_);
}

// -----------------------------------------------------------------------------
float MaxK_Array::add(              // add a key
    float key)                          // input key
{
    int i = 0;
    for (i = num_; i > 0; --i) {
        if (keys_[i-1] < key) keys_[i] = keys_[i-1];
        else break;
    }
    keys_[i] = key;                 // store new item
    if (num_ < k_) ++num_;          // increase the number of items

    return min_key();
}

} // end namespace ip
