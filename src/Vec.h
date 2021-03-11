/*******************************************************************************************[Vec.h]
Copyright (c) 2003-2007, Niklas Een, Niklas Sorensson
Copyright (c) 2007-2010, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#ifndef Vec_h
#define Vec_h

#ifdef USE_FOLLY
#include <folly/FBVector.h>
#endif
#include <iostream>

#include <cassert>
#include <cstdint>
#include <limits>
#include <new>
#include <utility>

#include "XAlloc.h"

// #define DEBUG_VEC

#ifdef DEBUG_VEC
#define ASSERT_VEC(x) assert(x)
#ifdef USE_FOLLY
#define TRACE_VEC                                          \
    uint32_t orig_size = this->folly::fbvector<T>::size(); \
    std::cout << __FUNCTION__ << " " << this->folly::fbvector<T>::size() << std::endl;
#else
#define TRACE_VEC std::cout << __FUNCTION__ << " " << this->size() << std::endl
#endif
#else
#define ASSERT_VEC(x)
#define TRACE_VEC
#endif

namespace CMSat {

class Watched;

//=================================================================================================
// Automatically resizable arrays
//
// NOTE! Don't use this vector on datatypes that cannot be re-located in memory (with realloc)
//
//

#ifdef USE_FOLLY
template <class T>
class vec : public folly::fbvector<T>
{
   public:
    void growTo(uint32_t size)
    {
        TRACE_VEC;
        if (this->size() > size)
            return;
        this->resize(size);
        ASSERT_VEC(this->size() == size);
    }
    void growTo(uint32_t size, const T& pad)
    {
        TRACE_VEC;
        if (this->size() > size)
            return;
        this->resize(size, pad);
        ASSERT_VEC(this->size() == size);
    }
    void shrink(uint32_t nelems)
    {
        TRACE_VEC;
        ASSERT_VEC(nelems <= orig_size);
        this->erase(this->end() - nelems, this->end());
        this->folly::fbvector<T>::shrink_to_fit();
        ASSERT_VEC(this->size() == (orig_size - nelems));
    }
    void insert(uint32_t num)
    {
        TRACE_VEC;
        growTo(this->size() + num);
        ASSERT_VEC(this->size() == (orig_size + num));
    }
    void push()
    {
        TRACE_VEC;
        this->emplace_back(T());
        ASSERT_VEC(this->size() == (orig_size + 1));
    }
    void push(const T& elem)
    {
        TRACE_VEC;
        this->push_back(elem);
        ASSERT_VEC(this->size() == (orig_size + 1));
    }
    const T& last() const
    {
        TRACE_VEC;
        return this->back();
    }
    void pop()
    {
        TRACE_VEC;
        this->pop_back();
        ASSERT_VEC(this->size() == (orig_size - 1));
    }
    void clear(bool dealloc = false)
    {
        TRACE_VEC;
        (void)dealloc;
        ASSERT_VEC(!dealloc);
        this->folly::fbvector<T>::clear();
        this->folly::fbvector<T>::shrink_to_fit();
        ASSERT_VEC(this->folly::fbvector<T>::size() == 0);
        ASSERT_VEC(this->folly::fbvector<T>::capacity() == 0);
    }
    void shrink_(uint32_t nelems)
    {
        TRACE_VEC;
        ASSERT_VEC(nelems <= orig_size);
        this->erase(this->end() - nelems, this->end());
        this->folly::fbvector<T>::shrink_to_fit();
        ASSERT_VEC(this->size() == (orig_size - nelems));
    }
    void moveTo(vec<T>& dest)
    {
        TRACE_VEC;
        dest.clear(true);
        dest = *this;
        clear();
        ASSERT_VEC(orig_size == dest.folly::fbvector<T>::size());
        ASSERT_VEC(this->size() == 0);
    }
};

/* ^^^ USE_FOLLY ^^^ */
#else
/* vvv !USE_FOLLY vvv */

template<class T>
class vec {
public:
    T*  data;
    T* begin()
    {
        return data;
    }
    T* end()
    {
        return data + sz;
    }

    const T* begin() const
    {
        return data;
    }
    const T* end() const
    {
        return data + sz;
    }
private:
    uint32_t sz;
    uint32_t cap;

    // Don't allow copying (error prone):
    vec<T>&  operator = (vec<T>& /*other*/)
    {
        assert(0);
        return *this;
    }
    vec      (vec<T>& /*other*/)
    {
        assert(0);
    }

    // Helpers for calculating next capacity:
    static inline uint32_t  imax   (int32_t x, int32_t y)
    {
        int32_t mask = (y - x) >> (sizeof(uint32_t) * 8 - 1);
        return (x & mask) + (y & (~mask));
    }

public:
    // Constructors:
    vec()                       : data(NULL) , sz(0)   , cap(0)    { }
    ~vec()
    {
        // clear(true);
    }

    // Size operations:
    uint32_t      size() const
    {
        return sz;
    }
    void     shrink   (uint32_t nelems)
    {
        TRACE_VEC;
        assert(nelems <= sz);
        for (uint32_t i = 0; i < nelems; i++) {
            sz--, data[sz].~T();
        }
    }
    void     shrink_  (uint32_t nelems)
    {
        TRACE_VEC;
        assert(nelems <= sz);
        sz -= nelems;
    }
    uint32_t      capacity () const
    {
        return cap;
    }
    void     capacity (int32_t min_cap);
    void     growTo   (uint32_t size);
    void     growTo   (uint32_t size, const T& pad);
    void     clear    (bool dealloc = false);

    // Stack interface:
    void     push  ()
    {
        if (sz == cap) {
            capacity(sz + 1);
        }
        new (&data[sz]) T();
        sz++;
    }
    void     push  (const T& elem)
    {
        TRACE_VEC;
        if (sz == cap) {
            capacity(sz + 1);
        }
        data[sz++] = elem;
    }
    void     push_ (const T& elem)
    {
        assert(sz < cap);
        data[sz++] = elem;
    }
    void     pop   ()
    {
        assert(sz > 0);
        sz--, data[sz].~T();
    }
    // NOTE: it seems possible that overflow can happen in the 'sz+1' expression of 'push()', but
    // in fact it can not since it requires that 'cap' is equal to INT_MAX. This in turn can not
    // happen given the way capacities are calculated (below). Essentially, all capacities are
    // even, but INT_MAX is odd.

    const T& last  () const
    {
        return data[sz - 1];
    }
    T&       last  ()
    {
        return data[sz - 1];
    }

    // Vector interface:
    const T& operator [] (uint32_t index) const
    {
        return data[index];
    }
    T&       operator [] (uint32_t index)
    {
        return data[index];
    }

    // Duplicatation (preferred instead):
    void copyTo(vec<T>& copy) const
    {
        TRACE_VEC;
        copy.clear();
        copy.growTo(sz);
        for (uint32_t i = 0; i < sz; i++) {
            copy[i] = data[i];
        }
    }
    void moveTo(vec<T>& dest)
    {
        TRACE_VEC;
        dest.clear(true);
        dest.data = data;
        dest.sz = sz;
        dest.cap = cap;
        data = NULL;
        sz = 0;
        cap = 0;
    }
    void swap(vec<T>& dest)
    {
        TRACE_VEC;
        std::swap(dest.data, data);
        std::swap(dest.sz, sz);
        std::swap(dest.cap, cap);
    }

    void resize(uint32_t s) {
        TRACE_VEC;
        if (s < sz) {
            shrink(sz - s);
        } else {
            growTo(s);
        }
    }

    void insert(uint32_t num)
    {
        TRACE_VEC;
        growTo(sz+num);
    }

    bool empty() const
    {
        return sz == 0;
    }

    void shrink_to_fit()
    {
        if (sz == 0) {
            free(data);
            cap = 0;
            data = NULL;
            return;
        }

        T* data2 = (T*)realloc(data, sz*sizeof(T));
        if (data2 == 0) {
            //We just keep the size then
            return;
        }
        data = data2;
        cap = sz;
     }
};


// Fixes by @Topologist from GitHub. Thank you so much!
template<class T>
void vec<T>::capacity(int32_t min_cap)
{
    if ((int32_t)cap >= min_cap) {
        return;
    }

    // NOTE: grow by approximately 3/2
    uint32_t add = imax((min_cap - (int32_t)cap + 1) & ~1, (((int32_t)cap >> 1) + 2) & ~1);
    if (add > std::numeric_limits<uint32_t>::max() - cap) {
        throw std::bad_alloc();
    }
    cap += (uint32_t)add;

    // This avoids memory fragmentation by many reallocations
    uint32_t new_size = 2;
    while (new_size < cap) {
        new_size *= 2;
    }
    if (new_size * 2 / 3 > cap) {
        new_size = new_size * 2 / 3;
    }
    cap = new_size;

    if (((data = (T*)::realloc(data, cap * sizeof(T))) == NULL) && errno == ENOMEM) {
        throw std::bad_alloc();
    }
}


template<class T>
void vec<T>::growTo(uint32_t size, const T& pad)
{
    TRACE_VEC;
    if (sz >= size) {
        return;
    }
    capacity(size);
    for (uint32_t i = sz; i < size; i++) {
        data[i] = pad;
    }
    sz = size;
}


template<class T>
void vec<T>::growTo(uint32_t size)
{
    TRACE_VEC;
    if (sz >= size) {
        return;
    }
    capacity(size);
    for (uint32_t i = sz; i < size; i++) {
        new (&data[i]) T();
    }
    sz = size;
}


template<class T>
void vec<T>::clear(bool dealloc)
{
    TRACE_VEC;
    if (data != NULL) {
        for (uint32_t i = 0; i < sz; i++) {
            data[i].~T();
        }
        sz = 0;
        if (dealloc) {
            free(data), data = NULL, cap = 0;
        }
    }
}

template<>
inline void vec<Watched>::clear(bool dealloc)
{
    TRACE_VEC;
    if (data != NULL) {
        sz = 0;
        if (dealloc) {
            free(data), data = NULL, cap = 0;
        }
    }
}
/* ^^^ !USE_FOLLY ^^^ */
#endif

//=================================================================================================
} // namespace CMSat

#endif
