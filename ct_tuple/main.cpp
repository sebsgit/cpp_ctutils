#include "ct_tuple.hpp"
#include <iostream>

int main()
{
    constexpr tuple_utils::ct_tuple<int> c0(9);
    static_assert(c0.get() == 9, "");
    constexpr auto c0_d = c0.add(3.14f);
    static_assert(c0_d.get() == 9, "");
    static_assert(c0_d.tail().get() == 3.14f, "");
    constexpr tuple_utils::ct_tuple<double, int> c1(10.4, 55);
    static_assert(c1.get() == 10.4, "");
    static_assert(c1.tail().get() == 55, "");

    constexpr tuple_utils::ct_tuple<double, float, int, char> data(4.5, 55.1f, -21, 'z');
    constexpr auto data2 = data.add(77);
    static_assert(data2.get() == 4.5, "");
    static_assert(data2.tail().get() == 55.1f, "");
    static_assert(data2.tail().tail().get() == -21, "");
    static_assert(data2.tail().tail().tail().get() == 'z', "");
    static_assert(data2.tail().tail().tail().tail().get() == 77, "");

    static_assert(tuple_utils::get<0>(data2) == 4.5, "");
    static_assert(tuple_utils::get<1>(data2) == 55.1f, "");
    static_assert(tuple_utils::get<2>(data2) == -21, "");
    static_assert(tuple_utils::get<3>(data2) == 'z', "");
    static_assert(tuple_utils::get<4>(data2) == 77, "");
}
