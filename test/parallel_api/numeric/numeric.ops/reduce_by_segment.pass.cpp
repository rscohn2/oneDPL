// -*- C++ -*-
//===-- reduce_by_segment.pass.cpp ----------------------------------------===//
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

#include <complex>

#include "oneapi/dpl/execution"
#include "oneapi/dpl/algorithm"
#include "oneapi/dpl/iterator"
#include "oneapi/dpl/complex"

#include "support/test_config.h"
#include "support/utils.h"
#include "support/reduce_serial_impl.h"

#include <iostream>
#include <iomanip>

#if TEST_DPCPP_BACKEND_PRESENT
#    include "support/utils_sycl.h"

using namespace oneapi::dpl::execution;
#endif // TEST_DPCPP_BACKEND_PRESENT
using namespace TestUtils;

// This macro may be used to analyze source data and test results in test_reduce_by_segment
// WARNING: in the case of using this macro debug output is very large.
// #define DUMP_CHECK_RESULTS

DEFINE_TEST_2(test_reduce_by_segment, BinaryPredicate, BinaryOperation)
{
    DEFINE_TEST_CONSTRUCTOR(test_reduce_by_segment)

    template <typename Iterator1, typename Iterator2, typename Iterator3, typename Iterator4, typename Size>
    void initialize_data(Iterator1 host_keys, Iterator2 host_vals, Iterator3 host_key_res, Iterator4 host_val_res,
                         Size n)
    {
        // T keys[n] = { 1, 2, 3, 4, 1, 1, 2, 2, 3, 3, 4, 4, 1, 1, 1, ...};
        // T vals[n] = { n random numbers between 0 and 4 };

        ::std::srand(42);
        Size segment_length = 1;
        for (Size i = 0; i != n;)
        {
            for (Size j = 0; j != 4 * segment_length && i != n; ++j)
            {
                host_keys[i] = j / segment_length + 1;
                host_vals[i] = rand() % 5;
                host_key_res[i] = 0;
                host_val_res[i] = 0;
                ++i;
            }
            ++segment_length;
        }
    }

#ifdef DUMP_CHECK_RESULTS
    template <typename Iterator, typename Size>
    void display_param(const char* msg, Iterator it, Size n)
    {
        ::std::cout << msg;
        for (Size i = 0; i < n; ++i)
        {
            if (i > 0)
                ::std::cout << ", ";
            ::std::cout << it[i];
        }
        ::std::cout << ::std::endl;
    }
#endif // DUMP_CHECK_RESULTS

    template <typename Iterator1, typename Iterator2, typename Iterator3, typename Iterator4, typename InputSize,
              typename OutputKeySize, typename OutputValSize,
              typename BinaryPredicateCheck = oneapi::dpl::__internal::__pstl_equal,
              typename BinaryOperationCheck = oneapi::dpl::__internal::__pstl_plus>
    void check_values(Iterator1 host_keys, Iterator2 host_vals, Iterator3 key_res, Iterator4 val_res, InputSize n,
                      OutputKeySize num_keys, OutputValSize num_vals,
                      BinaryPredicateCheck pred = BinaryPredicateCheck(),
                      BinaryOperationCheck op = BinaryOperationCheck())
    {
        // https://docs.oneapi.io/versions/latest/onedpl/extension_api.html
        // keys:   [ 0, 0, 0, 1, 1, 1 ]
        // values: [ 1, 2, 3, 4, 5, 6 ]
        // result keys: [ 0, 1 ]
        // result values: [ 1 + 2 + 3 = 6, 4 + 5 + 6 = 15 ]

        if (n < 1)
            return;

        typedef typename ::std::iterator_traits<Iterator1>::value_type KeyT;
        typedef typename ::std::iterator_traits<Iterator2>::value_type ValT;

        ::std::string failure_msg = "from reduce_by_segment with types [";
        failure_msg += typeid(KeyT).name();
        failure_msg += ";";
        failure_msg += typeid(ValT).name();
        failure_msg += "], ";
        failure_msg += "n=" + ::std::to_string(n) + ", ";
        if (::std::is_same<BinaryPredicateCheck, oneapi::dpl::__internal::__pstl_equal>::value)
        {
            failure_msg += "Default Pred, ";
        }
        else
        {
            failure_msg += "Custom Pred, ";
        }
        if (::std::is_same<BinaryOperationCheck, oneapi::dpl::__internal::__pstl_plus>::value)
        {
            failure_msg += "Default Op";
        }
        else
        {
            failure_msg += "Custom Op";
        }
        ::std::string num_keys_failure_msg = "wrong key size " + failure_msg;
        ::std::string num_vals_failure_msg = "wrong values size " + failure_msg;
        ::std::string key_failure_msg = "wrong keys " + failure_msg;
        ::std::string val_failure_msg = "wrong values " + failure_msg;

        ::std::vector<KeyT> expected_key_res(n);
        ::std::vector<ValT> expected_val_res(n);

        ::std::size_t num_segments =
            reduce_by_segment_serial(host_keys, host_vals, expected_key_res, expected_val_res, n, pred, op);

#ifdef DUMP_CHECK_RESULTS
        ::std::cout << "check_values(n = " << n << ", segments = " << num_keys << ") : " << ::std::endl;
        display_param("           keys: ", host_keys, n);
        display_param("         values: ", host_vals, n);
        display_param("    result keys: ", key_res, num_keys);
        display_param("  result values: ", val_res, num_vals);
        display_param("  expected keys: ", expected_key_res.data(), num_segments);
        display_param("expected values: ", expected_val_res.data(), num_segments);
#endif // DUMP_CHECK_RESULTS

        EXPECT_EQ(num_segments, num_keys, num_keys_failure_msg.c_str());
        EXPECT_EQ(num_segments, num_vals, num_vals_failure_msg.c_str());
        EXPECT_EQ_N(expected_key_res.data(), key_res, num_keys, key_failure_msg.c_str());
        EXPECT_EQ_N(expected_val_res.data(), val_res, num_vals, val_failure_msg.c_str());
    }

#if TEST_DPCPP_BACKEND_PRESENT
    // specialization for hetero policy
    template <typename Policy, typename Iterator1, typename Iterator2, typename Iterator3, typename Iterator4,
              typename Size>
    typename ::std::enable_if<
        oneapi::dpl::__internal::__is_hetero_execution_policy<typename ::std::decay<Policy>::type>::value &&
            is_base_of_iterator_category<::std::random_access_iterator_tag, Iterator3>::value &&
            is_base_of_iterator_category<::std::random_access_iterator_tag, Iterator4>::value,
        void>::type
    operator()(Policy&& exec, Iterator1 keys_first, Iterator1 keys_last, Iterator2 vals_first, Iterator2 vals_last,
               Iterator3 key_res_first, Iterator3 key_res_last, Iterator4 val_res_first, Iterator4 val_res_last, Size n)
    {
        TestDataTransfer<UDTKind::eKeys, Size> host_keys(*this, n);
        TestDataTransfer<UDTKind::eVals, Size> host_vals(*this, n);
        TestDataTransfer<UDTKind::eResKeys, Size> host_res_keys(*this, n);
        TestDataTransfer<UDTKind::eRes, Size> host_res(*this, n);

        typedef typename ::std::iterator_traits<Iterator1>::value_type KeyT;

        // call algorithm with no optional arguments
        initialize_data(host_keys.get(), host_vals.get(), host_res_keys.get(), host_res.get(), n);
        update_data(host_keys, host_vals, host_res_keys, host_res);

        auto new_policy = make_new_policy<new_kernel_name<Policy, 0>>(exec);
        auto res1 =
            oneapi::dpl::reduce_by_segment(new_policy, keys_first, keys_last, vals_first, key_res_first, val_res_first);
        exec.queue().wait_and_throw();

        retrieve_data(host_keys, host_vals, host_res_keys, host_res);
        size_t segments_key_ret1 = ::std::distance(key_res_first, res1.first);
        size_t segments_val_ret1 = ::std::distance(val_res_first, res1.second);
        check_values(host_keys.get(), host_vals.get(), host_res_keys.get(), host_res.get(), n, segments_key_ret1,
                     segments_val_ret1);

        // call algorithm with predicate
        initialize_data(host_keys.get(), host_vals.get(), host_res_keys.get(), host_res.get(), n);
        update_data(host_keys, host_vals, host_res_keys, host_res);

        auto new_policy2 = make_new_policy<new_kernel_name<Policy, 1>>(exec);
        auto res2 = oneapi::dpl::reduce_by_segment(new_policy2, keys_first, keys_last, vals_first, key_res_first,
                                                   val_res_first, BinaryPredicate());
        exec.queue().wait_and_throw();

        retrieve_data(host_keys, host_vals, host_res_keys, host_res);
        size_t segments_key_ret2 = ::std::distance(key_res_first, res2.first);
        size_t segments_val_ret2 = ::std::distance(val_res_first, res2.second);
        check_values(host_keys.get(), host_vals.get(), host_res_keys.get(), host_res.get(), n, segments_key_ret2,
                     segments_val_ret2, BinaryPredicate());

        // call algorithm with predicate and operator
        initialize_data(host_keys.get(), host_vals.get(), host_res_keys.get(), host_res.get(), n);
        update_data(host_keys, host_vals, host_res_keys, host_res);

        auto new_policy3 = make_new_policy<new_kernel_name<Policy, 2>>(exec);
        auto res3 = oneapi::dpl::reduce_by_segment(new_policy3, keys_first, keys_last, vals_first, key_res_first,
                                                   val_res_first, BinaryPredicate(), BinaryOperation());
        exec.queue().wait_and_throw();

        retrieve_data(host_keys, host_vals, host_res_keys, host_res);
        size_t segments_key_ret3 = ::std::distance(key_res_first, res3.first);
        size_t segments_val_ret3 = ::std::distance(val_res_first, res3.second);
        check_values(host_keys.get(), host_vals.get(), host_res_keys.get(), host_res.get(), n, segments_key_ret3,
                     segments_val_ret3, BinaryPredicate(), BinaryOperation());
    }
#endif

    // specialization for host execution policies
    template <typename Policy, typename Iterator1, typename Iterator2, typename Iterator3, typename Iterator4,
              typename Size>
    typename ::std::enable_if<
#if TEST_DPCPP_BACKEND_PRESENT
        !oneapi::dpl::__internal::__is_hetero_execution_policy<typename ::std::decay<Policy>::type>::value &&
#endif
            is_base_of_iterator_category<::std::random_access_iterator_tag, Iterator3>::value &&
            is_base_of_iterator_category<::std::random_access_iterator_tag, Iterator4>::value,
        void>::type
    operator()(Policy&& exec, Iterator1 keys_first, Iterator1 keys_last, Iterator2 vals_first, Iterator2 vals_last,
               Iterator3 key_res_first, Iterator3 key_res_last, Iterator4 val_res_first, Iterator4 val_res_last, Size n)
    {
        // call algorithm with no optional arguments
        initialize_data(keys_first, vals_first, key_res_first, val_res_first, n);
        auto res1 =
            oneapi::dpl::reduce_by_segment(exec, keys_first, keys_last, vals_first, key_res_first, val_res_first);
        size_t segments_key_ret1 = ::std::distance(key_res_first, res1.first);
        size_t segments_val_ret1 = ::std::distance(val_res_first, res1.second);
        check_values(keys_first, vals_first, key_res_first, val_res_first, n, segments_key_ret1, segments_val_ret1);

        // call algorithm with predicate
        initialize_data(keys_first, vals_first, key_res_first, val_res_first, n);
        auto res2 = oneapi::dpl::reduce_by_segment(exec, keys_first, keys_last, vals_first, key_res_first,
                                                   val_res_first, BinaryPredicate());
        size_t segments_key_ret2 = ::std::distance(key_res_first, res2.first);
        size_t segments_val_ret2 = ::std::distance(val_res_first, res2.second);
        check_values(keys_first, vals_first, key_res_first, val_res_first, n, segments_key_ret2, segments_val_ret2,
                     BinaryPredicate());

        // call algorithm with predicate and operator
        initialize_data(keys_first, vals_first, key_res_first, val_res_first, n);
        auto res3 = oneapi::dpl::reduce_by_segment(exec, keys_first, keys_last, vals_first, key_res_first,
                                                   val_res_first, BinaryPredicate(), BinaryOperation());
        size_t segments_key_ret3 = ::std::distance(key_res_first, res3.first);
        size_t segments_val_ret3 = ::std::distance(val_res_first, res3.second);
        check_values(keys_first, vals_first, key_res_first, val_res_first, n, segments_key_ret3, segments_val_ret3,
                     BinaryPredicate(), BinaryOperation());
    }

    // specialization for non-random_access iterators
    template <typename Policy, typename Iterator1, typename Iterator2, typename Iterator3, typename Iterator4,
              typename Size>
    typename ::std::enable_if<!is_base_of_iterator_category<::std::random_access_iterator_tag, Iterator3>::value ||
                                  !is_base_of_iterator_category<::std::random_access_iterator_tag, Iterator4>::value,
                              void>::type
    operator()(Policy&& exec, Iterator1 keys_first, Iterator1 keys_last, Iterator2 vals_first, Iterator2 vals_last,
               Iterator3 key_res_first, Iterator3 key_res_last, Iterator4 val_res_first, Iterator4 val_res_last, Size n)
    {
    }
};

template <typename _Tp>
struct UserBinaryPredicate
{
    bool
    operator()(const _Tp& __x, const _Tp& __y) const
    {
        using KeyT = ::std::decay_t<decltype(__y)>;
        return __y != KeyT(1);
    }
};

template <typename _Tp>
struct UserBinaryOperation
{
    _Tp
    operator()(const _Tp& __x, const _Tp& __y) const
    {
        return __x * __y;
    }
};

int
main()
{
    {
        using ValueType = ::std::uint64_t;
        using BinaryPredicate = UserBinaryPredicate<ValueType>;
        using BinaryOperation = UserBinaryOperation<ValueType>;

#if TEST_DPCPP_BACKEND_PRESENT
        // Run tests for USM shared memory
        test4buffers<sycl::usm::alloc::shared, test_reduce_by_segment<ValueType, BinaryPredicate, BinaryOperation>>();
        // Run tests for USM device memory
        test4buffers<sycl::usm::alloc::device, test_reduce_by_segment<ValueType, BinaryPredicate, BinaryOperation>>();
#endif // TEST_DPCPP_BACKEND_PRESENT

#if !_PSTL_ICC_TEST_SIMD_UDS_BROKEN
#    if TEST_DPCPP_BACKEND_PRESENT
        test_algo_four_sequences<test_reduce_by_segment<ValueType, BinaryPredicate, BinaryOperation>>();
#else
        test_algo_four_sequences<ValueType, test_reduce_by_segment<BinaryPredicate, BinaryOperation>>();
#    endif // TEST_DPCPP_BACKEND_PRESENT
#endif     // !_PSTL_ICC_TEST_SIMD_UDS_BROKEN
    }

    {
        using ValueType = ::std::complex<float>;
        using BinaryPredicate = UserBinaryPredicate<ValueType>;
        using BinaryOperation = UserBinaryOperation<ValueType>;

#if TEST_DPCPP_BACKEND_PRESENT
        // Run tests for USM shared memory
        test4buffers<sycl::usm::alloc::shared, test_reduce_by_segment<ValueType, BinaryPredicate, BinaryOperation>>();
        // Run tests for USM device memory
        test4buffers<sycl::usm::alloc::device, test_reduce_by_segment<ValueType, BinaryPredicate, BinaryOperation>>();
#endif // TEST_DPCPP_BACKEND_PRESENT

#if !_PSTL_ICC_TEST_SIMD_UDS_BROKEN
#    if TEST_DPCPP_BACKEND_PRESENT
        test_algo_four_sequences<test_reduce_by_segment<ValueType, BinaryPredicate, BinaryOperation>>();
#else
        test_algo_four_sequences<ValueType, test_reduce_by_segment<BinaryPredicate, BinaryOperation>>();
#    endif // TEST_DPCPP_BACKEND_PRESENT
#endif     // !_PSTL_ICC_TEST_SIMD_UDS_BROKEN
    }

    return TestUtils::done();
}
