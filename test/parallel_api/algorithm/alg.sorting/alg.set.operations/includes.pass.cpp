// -*- C++ -*-
//===-- includes.pass.cpp -------------------------------------------------===//
//
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// This file incorporates work covered by the following copyright and permission
// notice:
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
//
//===----------------------------------------------------------------------===//

#include "support/pstl_test_config.h"

#include _PSTL_TEST_HEADER(execution)
#include _PSTL_TEST_HEADER(algorithm)

#include "support/utils.h"

#include <cmath>

using namespace TestUtils;

template <typename T>
struct Num
{
    T val;
    explicit Num(const T& v) : val(v) {}

    //for "includes" checks
    template <typename T1>
    bool
    operator<(const Num<T1>& v1) const
    {
        return val < v1.val;
    }

    //The types Type1 and Type2 must be such that an object of type InputIt can be dereferenced and then implicitly converted to both of them
    template <typename T1>
    operator Num<T1>() const
    {
        return Num<T1>((T1)val);
    }
};

template <typename T>
struct test_one_policy
{
    template <typename Policy, typename InputIterator1, typename InputIterator2, typename Compare>
    typename ::std::enable_if<!TestUtils::isReverse<InputIterator1>::value, void>::type
    operator()(Policy&& exec, InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2,
               Compare comp)
    {

        auto expect_res = ::std::includes(first1, last1, first2, last2, comp);
        auto res = ::std::includes(exec, first1, last1, first2, last2, comp);

        EXPECT_TRUE(expect_res == res, "wrong result for includes");
    }

    template <typename Policy, typename InputIterator1, typename InputIterator2, typename Compare>
    typename ::std::enable_if<TestUtils::isReverse<InputIterator1>::value, void>::type
    operator()(Policy&& /* exec */, InputIterator1 /* first1 */, InputIterator1 /* last1 */, InputIterator2 /* first2 */, InputIterator2 /* last2 */,
               Compare /* comp */)
    {
    }
};

template <typename T1, typename T2, typename Compare>
void
test_includes(Compare compare)
{

    const ::std::size_t n_max = 1000000;

    // The rand()%(2*n+1) encourages generation of some duplicates.
    ::std::srand(42);

    for (::std::size_t n = 0; n < n_max; n = n <= 16 ? n + 1 : size_t(3.1415 * n))
    {
        for (::std::size_t m = 0; m < n_max; m = m <= 16 ? m + 1 : size_t(2.71828 * m))
        {
            //prepare the input ranges
            Sequence<T1> in1(n, [](::std::size_t k) { return rand() % (2 * k + 1); });
            Sequence<T2> in2(m, [](::std::size_t k) { return rand() % (k + 1); });

            ::std::sort(in1.begin(), in1.end(), compare);
            ::std::sort(in2.begin(), in2.end(), compare);

            invoke_on_all_policies<0>()(test_one_policy<T1>(), in1.begin(), in1.end(), in2.cbegin(), in2.cend(),
                                        compare);
            //test w/ non constant predicate
            if (n < 5 && m < 5)
                invoke_on_all_host_policies()(test_one_policy<T1>(), in1.begin(), in1.end(), in2.cbegin(),
                                              in2.cend(), non_const(compare));
        }
    }
}

int
main()
{

    test_includes<float64_t, float64_t>(oneapi::dpl::__internal::__pstl_less());
#if !_ONEDPL_BACKEND_SYCL
    test_includes<Num<int64_t>, Num<int32_t>>([](const Num<int64_t>& x, const Num<int32_t>& y) { return x < y; });
#endif

    ::std::cout << done() << ::std::endl;

    return 0;
}