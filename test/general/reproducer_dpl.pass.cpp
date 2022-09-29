#include <oneapi/dpl/execution>
#include <oneapi/dpl/algorithm>
#include <CL/sycl.hpp>
//#include <dpct/dpct.hpp>
//#include <dpct/dpl_utils.hpp>

namespace {
// template <typename _Tp> class constant_iterator {
// public:
//   typedef std::false_type is_hetero;
//   typedef std::true_type is_passed_directly;
//   typedef std::ptrdiff_t difference_type;
//   typedef _Tp value_type;
//   typedef _Tp *pointer;
//   // There is no storage behind the iterator, so we return a value instead of reference.
//   typedef const _Tp reference;
//   typedef const _Tp const_reference;
//   typedef std::random_access_iterator_tag iterator_category;

//   explicit constant_iterator(_Tp __init)
//       : __my_value_(__init), __my_counter_(0) {}
//   constant_iterator &operator= (const constant_iterator & __init) = default;

// private:
//   // used to construct iterator instances with different counter values required
//   // by arithmetic operators
//   constant_iterator(const _Tp &__value, const difference_type &__offset)
//       : __my_value_(__value), __my_counter_(__offset) {}

// public:
//   // non-const variants of access operators are not provided so unintended
//   // writes are caught at compile time.
//   const_reference operator*() const { return __my_value_; }
//   const_reference operator[](difference_type) const { return __my_value_; }

//   difference_type operator-(const constant_iterator &__it) const {
//     return __my_counter_ - __it.__my_counter_;
//   }

//   constant_iterator &operator+=(difference_type __forward) {
//     __my_counter_ += __forward;
//     return *this;
//   }
//   constant_iterator &operator-=(difference_type __backward) {
//     return *this += -__backward;
//   }
//   constant_iterator &operator++() { return *this += 1; }
//   constant_iterator &operator--() { return *this -= 1; }

//   constant_iterator operator++(int) {
//     constant_iterator __it(*this);
//     ++(*this);
//     return __it;
//   }
//   constant_iterator operator--(int) {
//     constant_iterator __it(*this);
//     --(*this);
//     return __it;
//   }

//   constant_iterator operator-(difference_type __backward) const {
//     return constant_iterator(__my_value_, __my_counter_ - __backward);
//   }
//   constant_iterator operator+(difference_type __forward) const {
//     return constant_iterator(__my_value_, __my_counter_ + __forward);
//   }
//   friend constant_iterator operator+(difference_type __forward,
//                                      const constant_iterator __it) {
//     return __it + __forward;
//   }

//   bool operator==(const constant_iterator &__it) const {
//     return __my_value_ == __it.__my_value_ &&
//            this->__my_counter_ == __it.__my_counter_;
//   }
//   bool operator!=(const constant_iterator &__it) const {
//     return !(*this == __it);
//   }
//   bool operator<(const constant_iterator &__it) const {
//     return *this - __it < 0;
//   }
//   bool operator>(const constant_iterator &__it) const { return __it < *this; }
//   bool operator<=(const constant_iterator &__it) const {
//     return !(*this > __it);
//   }
//   bool operator>=(const constant_iterator &__it) const {
//     return !(*this < __it);
//   }

// private:
//   _Tp __my_value_;
//   uint64_t __my_counter_;
// };

// template <typename _Tp>
// constant_iterator<_Tp> make_constant_iterator(_Tp __value) {
//   return constant_iterator<_Tp>(__value);
// }
}

template <typename index_t>
void
test(index_t* data, int num)
{
    auto stream = sycl::queue();
    auto sorted_data = dpct::device_pointer<index_t>(data);
    auto policy = oneapi::dpl::execution::make_device_policy(stream);
    oneapi::dpl::inclusive_scan_by_segment(policy, sorted_data, sorted_data + num, dpct::make_constant_iterator(1),
                                           sorted_data);

    oneapi::dpl::inclusive_scan_by_segment(
        policy, oneapi::dpl::make_reverse_iterator(sorted_data + num), oneapi::dpl::make_reverse_iterator(sorted_data),
        oneapi::dpl::make_reverse_iterator(sorted_data + num), oneapi::dpl::make_reverse_iterator(sorted_data + num),
        std::equal_to<index_t>(), oneapi::dpl::maximum<index_t>());
}

int
main()
{
    int* data = new int[10]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    test<int>(data, 10);
    int64_t* data64 = new int64_t[10]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    test<int64_t>(data64, 10);
    std::cout << "pass\n";
}