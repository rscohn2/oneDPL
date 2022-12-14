//===----------------------------------------------------------------------===//
//
// <array>
//
// tuple_size<array<T, N> >::value
//
//===----------------------------------------------------------------------===//

#include "oneapi_std_test_config.h"

#include <iostream>

#ifdef USE_ONEAPI_STD
#    include _ONEAPI_STD_TEST_HEADER(array)
namespace s = oneapi_cpp_ns;
#else
#    include <array>
namespace s = std;
#endif

#if TEST_DPCPP_BACKEND_PRESENT
template <class T, std::size_t N>
void
test()
{
    {
        typedef s::array<T, N> C;
        static_assert((s::tuple_size<C>::value == N), "");
    }
    {
        typedef s::array<T const, N> C;
        static_assert((s::tuple_size<C>::value == N), "");
    }
    {
        typedef s::array<T volatile, N> C;
        static_assert((s::tuple_size<C>::value == N), "");
    }
    {
        typedef s::array<T const volatile, N> C;
        static_assert((s::tuple_size<C>::value == N), "");
    }
}
#endif // TEST_DPCPP_BACKEND_PRESENT

int
main(int, char**)
{
#if TEST_DPCPP_BACKEND_PRESENT
    bool ret = false;
    {
        cl::sycl::buffer<bool, 1> buf(&ret, cl::sycl::range<1>{1});
        cl::sycl::queue q;
        q.submit([&](cl::sycl::handler& cgh) {
            auto ret_acc = buf.get_access<cl::sycl::access::mode::write>(cgh);
            cgh.single_task<class KernelTest1>([=]() {
                test<float, 0>();
                test<float, 3>();
                test<float, 5>();
                ret_acc[0] = true;
            });
        });
    }

    TestUtils::exitOnError(ret);
#endif // TEST_DPCPP_BACKEND_PRESENT

    return TestUtils::done(TEST_DPCPP_BACKEND_PRESENT);
}