//===----------------------------------------------------------------------===//
//
// template<class E> class initializer_list;
//
// const E* begin() const;
// const E* end() const;
// size_t size() const;
//
//===----------------------------------------------------------------------===//

#include "oneapi_std_test_config.h"
#include "test_macros.h"

#include <iostream>
#include <initializer_list>
#ifdef USE_ONEAPI_STD
#    include _ONEAPI_STD_TEST_HEADER(cstddef)
namespace s = oneapi_cpp_ns;
#else
#    include <cstddef>
namespace s = std;
#endif

#if TEST_DPCPP_BACKEND_PRESENT
struct A
{
    A(std::initializer_list<int> il)
    {
        const int* b = il.begin();
        const int* e = il.end();
        size = il.size();
        int i = 0;
        while (b < e)
        {
            data[i++] = *b++;
        }
    }

    s::size_t size;
    int data[10];
};
#endif // TEST_DPCPP_BACKEND_PRESENT

int
main(int, char**)
{
#if TEST_DPCPP_BACKEND_PRESENT
    const s::size_t N = 4;
    bool rs[N] = {false};

    {
        cl::sycl::buffer<bool, 1> buf(rs, cl::sycl::range<1>{N});
        cl::sycl::queue q;
        q.submit([&](cl::sycl::handler& cgh) {
            auto acc = buf.get_access<cl::sycl::access::mode::write>(cgh);
            cgh.single_task<class KernelTest1>([=]() {
                A test1 = {3, 2, 1};
                acc[0] = (test1.size == 3);
                acc[1] = (test1.data[0] == 3);
                acc[2] = (test1.data[1] == 2);
                acc[3] = (test1.data[2] == 1);
            });
        });
    }

    for (std::size_t i = 0; i < N; ++i)
    {
        if (!rs[i])
        {
            std::cout << "Fail" << std::endl;
            return -1;
        }
    }
#endif // TEST_DPCPP_BACKEND_PRESENT

    return TestUtils::done(TEST_DPCPP_BACKEND_PRESENT);
}