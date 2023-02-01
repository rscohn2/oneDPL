//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <complex>

// complex& operator-=(const complex& rhs);

// Warning	G2904547A	'complex' is deprecated: warning STL4037:
// The effect of instantiating the template std::complex for any type other than float, double, or long double is unspecified.
// You can define _SILENCE_NONFLOATING_COMPLEX_DEPRECATION_WARNING to suppress this warning. [-Wdeprecated-declarations]
#if defined(_MSC_VER)
#    define _SILENCE_NONFLOATING_COMPLEX_DEPRECATION_WARNING
#endif

#include "support/test_complex.h"

template <class T>
void
test()
{
    dpl::complex<T> c;
    const dpl::complex<T> c2(1.5f, 2.5f);
    assert(c.real() == 0);
    assert(c.imag() == 0);
    c -= c2;
    assert(c.real() == -1.5f);
    assert(c.imag() == -2.5f);
    c -= c2;
    assert(c.real() == -3);
    assert(c.imag() == -5);

    dpl::complex<T> c3;

#if !_PSTL_GLIBCXX_TEST_COMPLEX_MINUS_EQ_BROKEN
    c3 = c;
    dpl::complex<int> ic (1,1);
    c3 -= ic;
    assert(c3.real() == -4);
    assert(c3.imag() == -6);
#endif // !_PSTL_GLIBCXX_TEST_COMPLEX_MINUS_EQ_BROKEN

    c3 = c;
    dpl::complex<float> fc (1,1);
    c3 -= fc;
    assert(c3.real() == -4);
    assert(c3.imag() == -6);
}

ONEDPL_TEST_NUM_MAIN
{
    test<float>();
    IF_DOUBLE_SUPPORT(test<double>())
    IF_LONG_DOUBLE_SUPPORT(test<long double>())

  return 0;
}
