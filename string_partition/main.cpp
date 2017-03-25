#include "string_part.hpp"
#include "type_list.hpp"
#include "string_partition.hpp"
#include "partition_iterator.hpp"

#include <iostream>
#include <typeinfo>
#include <cassert>

static void test_type_list()
{
    using list = type_list::create<void>::type;
    using add_int = type_list::append<list, int>::type;
    using add_double = type_list::append<add_int, double>::type;
    static_assert(type_list::count<add_double>::size == 3, "size");
    static_assert(std::is_same<void, type_list::at<add_double, 0>::type>::value, "list[0] == void");
    static_assert(std::is_same<int, type_list::at<add_double, 1>::type>::value, "list[1] == int");
    static_assert(std::is_same<double, type_list::at<add_double, 2>::type>::value, "list[2] == double");
}

static void test_string_parts()
{
    constexpr auto filled_part = string_part::make_filed_part<16, 0, sizeof("two")>("two");
    static_assert(filled_part.size() == 16, "");
    static_assert(filled_part.data(0) == 't', "");
    static_assert(filled_part.data(1) == 'w', "");
    static_assert(filled_part.data(2) == 'o', "");
    static_assert(filled_part.data(3) == '\0', "");
    static_assert(filled_part.data(11) == '\0', "");
    static_assert(filled_part.data(15) == '\0', "");
}

static std::string _iteratorResult;

template <typename Part, typename Args>
class process_part {
public:
    auto operator() (Args&& args) const{
        _iteratorResult += Part(args).to_string();
        return 0;
    }
};

static void test_partition()
{
    constexpr auto part_length = 3;
    constexpr char test_data[] = "1234x5612x3456x1234x5678x";
    using parts = decltype(string_partition::make_partition<part_length>(test_data));
    partition_iterator::visit<parts, process_part>()(test_data);
    assert(_iteratorResult == std::string(test_data));
}

int main()
{
    test_type_list();
    test_string_parts();
    test_partition();
    return 0;
}
