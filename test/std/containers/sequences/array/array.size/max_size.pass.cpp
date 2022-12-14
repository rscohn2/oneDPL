// <array>

// class array

// bool max_size() const noexcept;

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
constexpr cl::sycl::access::mode sycl_read = cl::sycl::access::mode::read;
constexpr cl::sycl::access::mode sycl_write = cl::sycl::access::mode::write;
#endif // TEST_DPCPP_BACKEND_PRESENT

int
main(int, char**)
{
#if TEST_DPCPP_BACKEND_PRESENT
    {
        bool ret = true;
        {
            cl::sycl::queue myQueue;
            cl::sycl::range<1> numOfItems{1};
            cl::sycl::buffer<bool, 1> buf1(&ret, numOfItems);

            myQueue.submit([&](cl::sycl::handler& cgh) {
                auto ret_acc = buf1.get_access<sycl_write>(cgh);

                cgh.single_task<class KernelMaxSizeTest1>([=]() {
                    {
                        typedef s::array<int, 2> C;
                        C c;
                        (void)noexcept(c.max_size());
                        ret_acc[0] &= (c.max_size() == 2);
                    }
                    {
                        typedef s::array<int, 0> C;
                        C c;
                        (void)noexcept(c.max_size());
                        ret_acc[0] &= (c.max_size() == 0);
                    }
                });
            });
        }

        TestUtils::exitOnError(ret);
    }
#endif // TEST_DPCPP_BACKEND_PRESENT

    return TestUtils::done(TEST_DPCPP_BACKEND_PRESENT);
}