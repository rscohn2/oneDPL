#include "oneapi_std_test_config.h"

#include <iostream>

#ifdef USE_ONEAPI_STD
#    include _ONEAPI_STD_TEST_HEADER(array)
#    include _ONEAPI_STD_TEST_HEADER(memory)
namespace s = oneapi_cpp_ns;
#else
#    include <array>
namespace s = std;
#endif

#if TEST_DPCPP_BACKEND_PRESENT
void
test_contiguous()
{
    bool ret = true;
    {
        cl::sycl::queue myQueue;
        cl::sycl::buffer<bool, 1> buf(&ret, cl::sycl::range<1>(1));

        myQueue.submit([&](cl::sycl::handler& cgh) {
            auto ret_acc = buf.get_access<cl::sycl::access::mode::read_write>(cgh);

            cgh.single_task<class KernelContiguousTest>([=]() {
                typedef float T;
                typedef s::array<T, 3> C;
                C c = {1, 2, 3};
                for (size_t i = 0; i < c.size(); ++i)
                    ret_acc[0] &= (*(c.begin() + i) == *(s::addressof(*c.begin()) + i));
            });
        });
    }

    TestUtils::exitOnError(ret);
}
#endif // TEST_DPCPP_BACKEND_PRESENT

int
main(int, char**)
{
#if TEST_DPCPP_BACKEND_PRESENT
    test_contiguous();
#endif // TEST_DPCPP_BACKEND_PRESENT

    return TestUtils::done(TEST_DPCPP_BACKEND_PRESENT);
}