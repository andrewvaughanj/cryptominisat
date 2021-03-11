/******************************************
Copyright (C) 2009-2020 Authors of CryptoMiniSat, see AUTHORS file

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#ifndef _CARDFINDER_H_
#define _CARDFINDER_H_

#include "cms_vector.h"
#include <set>
#include <iostream>
#include <algorithm>
#include <set>
#include <limits>
#include "xor.h"
#include "cset.h"
#include "watcharray.h"

using std::set;

namespace CMSat {

class Solver;

class CardFinder
{

public:
    CardFinder(Solver* solver);
    void find_cards();
    const cms_vector<cms_vector<Lit>>& get_cards() const;

private:
    void get_vars_with_clash(const cms_vector<Lit>& lits, cms_vector<uint32_t>& vars) const;
    void find_pairwise_atmost1();
    void deal_with_clash(cms_vector<uint32_t>& vars);
    bool find_connector(Lit lit1, Lit lit2) const;
    std::string print_card(const cms_vector<Lit>& lits) const;
    void print_cards(const cms_vector<cms_vector<Lit>>& card_constraints) const;
    void find_two_product_atmost1();
    void clean_empty_cards();

    //from solver
    Solver* solver;
    cms_vector<uint16_t>& seen;
    cms_vector<uint8_t>& seen2;
    cms_vector<Lit>& toClear;

    //internal data
    cms_vector<cms_vector<Lit>> cards;
    uint64_t total_sizes = 0;
};

inline const cms_vector<cms_vector<Lit>>& CardFinder::get_cards() const
{
    return cards;
}

} //end namespace

#endif //_CARDFINDER_H_
