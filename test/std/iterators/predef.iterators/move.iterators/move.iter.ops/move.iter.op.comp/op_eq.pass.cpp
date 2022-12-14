//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <iterator>

// move_iterator

// template <InputIterator Iter1, InputIterator Iter2>
//   requires HasEqualTo<Iter1, Iter2>
//   bool
//   operator==(const move_iterator<Iter1>& x, const move_iterator<Iter2>& y);
//
//  constexpr in C++17

#include "oneapi_std_test_config.h"

#include <iostream>
#include "test_macros.h"
#include "test_iterators.h"

#ifdef USE_ONEAPI_STD
#    include _ONEAPI_STD_TEST_HEADER(iterator)
#    include _ONEAPI_STD_TEST_HEADER(type_traits)
namespace s = oneapi_cpp_ns;
#else
#    include <iterator>
#    include <type_traits>
namespace s = std;
#endif

#if TEST_DPCPP_BACKEND_PRESENT
constexpr cl::sycl::access::mode sycl_read = cl::sycl::access::mode::read;
constexpr cl::sycl::access::mode sycl_write = cl::sycl::access::mode::write;

template <class It>
bool
test(It l, It r, bool x)
{
    const s::move_iterator<It> r1(l);
    const s::move_iterator<It> r2(r);
    return ((r1 == r2) == x);
}

bool
kernel_test()
{
    cl::sycl::queue deviceQueue;
    cl::sycl::cl_bool ret = true;
    {
        cl::sycl::range<1> numOfItems{1};
        cl::sycl::buffer<cl::sycl::cl_bool, 1> buffer1(&ret, numOfItems);
        deviceQueue.submit([&](cl::sycl::handler& cgh) {
            auto ret_access = buffer1.get_access<sycl_write>(cgh);
            cgh.single_task<class KernelTest>([=]() {
                char s[] = "1234567890";
                ret_access[0] &= test(input_iterator<char*>(s), input_iterator<char*>(s), true);
                ret_access[0] &= test(input_iterator<char*>(s), input_iterator<char*>(s + 1), false);
                ret_access[0] &= test(forward_iterator<char*>(s), forward_iterator<char*>(s), true);
                ret_access[0] &= test(forward_iterator<char*>(s), forward_iterator<char*>(s + 1), false);
                ret_access[0] &= test(bidirectional_iterator<char*>(s), bidirectional_iterator<char*>(s), true);
                ret_access[0] &= test(bidirectional_iterator<char*>(s), bidirectional_iterator<char*>(s + 1), false);
                ret_access[0] &= test(random_access_iterator<char*>(s), random_access_iterator<char*>(s), true);
                ret_access[0] &= test(random_access_iterator<char*>(s), random_access_iterator<char*>(s + 1), false);
                ret_access[0] &= test(s, s, true);
                ret_access[0] &= test(s, s + 1, false);

#if TEST_STD_VER > 14
                {
                    constexpr const char* p = "123456789";
                    typedef s::move_iterator<const char*> MI;
                    constexpr MI it1 = s::make_move_iterator(p);
                    constexpr MI it2 = s::make_move_iterator(p + 5);
                    constexpr MI it3 = s::make_move_iterator(p);
                    static_assert(!(it1 == it2), "");
                    static_assert((it1 == it3), "");
                    static_assert(!(it2 == it3), "");
                }
#endif
            });
        });
    }
    return ret;
}
#endif // TEST_DPCPP_BACKEND_PRESENT

int
main(int, char**)
{
#if TEST_DPCPP_BACKEND_PRESENT
    auto ret = kernel_test();
    TestUtils::exitOnError(ret);
#endif // TEST_DPCPP_BACKEND_PRESENT

    return TestUtils::done(TEST_DPCPP_BACKEND_PRESENT);
}