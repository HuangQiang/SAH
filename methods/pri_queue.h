#pragma once

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>

#include "def.h"

namespace ip {

// -----------------------------------------------------------------------------
//  struct Result
// -----------------------------------------------------------------------------
struct Result {                     // basic structure
    float key_;                         // distance / random projection value
    int   id_;                          // element id
};

// -----------------------------------------------------------------------------
inline int cmp(                     // cmp func for lower_bound (ascending)
    const Result &a,                    // 1st element
    const Result &b)                    // 2nd element
{
    return a.key_ < b.key_;
}

// -----------------------------------------------------------------------------
inline int cmp_desc(                // cmp func for upper_bound (descending)
    const Result &a,                    // 1st element
    const Result &b)                    // 2nd element
{
    return a.key_ > b.key_;
}

// -----------------------------------------------------------------------------
int ResultComp(                     // compare function for qsort (ascending)
    const void *e1,                     // 1st element
    const void *e2);                    // 2nd element

// -----------------------------------------------------------------------------
int ResultCompDesc(                 // compare function for qsort (descending)
    const void *e1,                     // 1st element
    const void *e2);                    // 2nd element


// -----------------------------------------------------------------------------
//  MinK_List: a structure which maintains the smallest k values (of type float)
//  and associated element id (of type int).
//
//  This structure is often used for NNS, P2HNNS.
// -----------------------------------------------------------------------------
class MinK_List {
public:
    int k_;                         // max numner of keys
    int num_;                       // number of key current active
    Result *list_;                  // the list itself
    
    // -------------------------------------------------------------------------
    MinK_List(int k);               // constructor (given k size)
    
    // -------------------------------------------------------------------------
    ~MinK_List();                   // destructor

    // -------------------------------------------------------------------------
    float insert(                   // insert item
        float key,                      // key of item
        int   id);                      // id of item
    
    // -------------------------------------------------------------------------
    inline void reset() { num_ = 0; }

    // -------------------------------------------------------------------------
    inline float min_key() { return num_ > 0 ? list_[0].key_ : MAXREAL; }

    // -------------------------------------------------------------------------
    inline float max_key() { return num_ >= k_ ? list_[k_-1].key_ : MAXREAL; }

    // -------------------------------------------------------------------------
    inline float ith_key(int i) { return i < num_ ? list_[i].key_ : MAXREAL; }

    // -------------------------------------------------------------------------
    inline int ith_id(int i) { return i < num_ ? list_[i].id_ : MININT; }

    // -------------------------------------------------------------------------
    inline int size() { return num_; }

    // -------------------------------------------------------------------------
    inline bool isFull() { if (num_ >= k_) return true; else return false; }
};


// -----------------------------------------------------------------------------
//  MaxK_List: a structure which maintains the largest k values (of type float) 
//  and associated element id (of type int).
//
//  This structure is often used for MIPS, Reverse MIPS, FNS.
// -----------------------------------------------------------------------------
class MaxK_List {
public:
    int k_;                         // max numner of keys
    int num_;                       // number of key current active
    Result *list_;                  // the list itself
    
    // -------------------------------------------------------------------------
    MaxK_List(int k);               // constructor (given k size)
    
    // -------------------------------------------------------------------------
    ~MaxK_List();                   // destructor

    // -------------------------------------------------------------------------
    float insert(                   // insert item
        float key,                      // key of item
        int   id);                      // id of item

    // -------------------------------------------------------------------------
    inline void reset() { num_ = 0; }

    // -------------------------------------------------------------------------
    inline float max_key() { return num_ > 0 ? list_[0].key_ : MINREAL; }

    // -------------------------------------------------------------------------
    inline float min_key() { return num_ >= k_ ? list_[k_-1].key_ : MINREAL; }

    // -------------------------------------------------------------------------
    inline float ith_key(int i) { return i < num_ ? list_[i].key_ : MINREAL; }

    // -------------------------------------------------------------------------
    inline int ith_id(int i) { return i < num_ ? list_[i].id_ : MININT; }

    // -------------------------------------------------------------------------
    inline int size() { return num_; }

    // -------------------------------------------------------------------------
    inline bool isFull() { if (num_ >= k_) return true; else return false; }
};

// -----------------------------------------------------------------------------
//  MaxK_Array: a structure which maintains the largest k values (of type float) 
//
//  This structure is often used for MIPS, Reverse MIPS, FNS.
// -----------------------------------------------------------------------------
class MaxK_Array {
public:
    int   k_;                       // max numner of keys
    int   num_;                     // number of keys current active
    float *keys_;                   // keys
    
    // -------------------------------------------------------------------------
    MaxK_Array(int k);              // constructor (given k size)
    
    // -------------------------------------------------------------------------
    ~MaxK_Array();                  // destructor

    // -------------------------------------------------------------------------
    void init(                      // init with an sorted array
        int   k,                        // number of keys
        const float *keys);             // input keys
    
    // -------------------------------------------------------------------------
    float add(                      // add a key
        float key);                     // input key

    // -------------------------------------------------------------------------
    inline void reset() { num_ = 0; }

    // -------------------------------------------------------------------------
    inline float max_key() { return num_ > 0 ? keys_[0] : MINREAL; }

    // -------------------------------------------------------------------------
    inline float min_key() { return num_ >= k_ ? keys_[k_-1] : MINREAL; }

    // -------------------------------------------------------------------------
    inline float ith_key(int i) { return i < num_ ? keys_[i] : MINREAL; }

    // -------------------------------------------------------------------------
    inline int size() { return num_; }

    // -------------------------------------------------------------------------
    inline bool isFull() { return num_ >= k_; }
};

} // end namespace ip
