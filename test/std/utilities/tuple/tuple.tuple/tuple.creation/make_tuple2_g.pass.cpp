#include "oneapi_std_test_config.h"
#include "test_macros.h"

#include <iostream>

#ifdef USE_ONEAPI_STD
#    include _ONEAPI_STD_TEST_HEADER(tuple)
#    include _ONEAPI_STD_TEST_HEADER(utility)
namespace s = oneapi_cpp_ns;
#else
#    include <tuple>
#    include <utility>
namespace s = std;
#endif

#if TEST_DPCPP_BACKEND_PRESENT
constexpr cl::sycl::access::mode sycl_read = cl::sycl::access::mode::read;
constexpr cl::sycl::access::mode sycl_write = cl::sycl::access::mode::write;

struct A
{
    template <typename T>
    A(T)
    {
    }

    A(const A&) = default;
    A(A&&) = default;
    A&
    operator=(const A&) = default;
    A&
    operator=(A&&) = default;
    ~A() = default;
};
#endif // TEST_DPCPP_BACKEND_PRESENT

int
main()
{
#if TEST_DPCPP_BACKEND_PRESENT
    cl::sycl::queue deviceQueue;
    cl::sycl::cl_bool ret = false;
    cl::sycl::range<1> numOfItems{1};
    {
        cl::sycl::buffer<cl::sycl::cl_bool, 1> buffer1(&ret, numOfItems);
        deviceQueue.submit([&](cl::sycl::handler& cgh) {
            auto ret_access = buffer1.get_access<sycl_write>(cgh);
            cgh.single_task<class KernelTest>([=]() {
                {
                    auto x = A{7};
                    s::make_tuple(x);
                }
                ret_access[0] = true;
            });
        });
    }

    if (ret)
    {
        std::cout << "pass" << std::endl;
    }
    else
    {
        std::cout << "fail" << std::endl;
    }
#endif // TEST_DPCPP_BACKEND_PRESENT

    return TestUtils::done(TEST_DPCPP_BACKEND_PRESENT);
}