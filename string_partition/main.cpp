#include "string_part.hpp"
#include "type_list.hpp"
#include "string_partition.hpp"
#include "partition_iterator.hpp"
#include "partition_converter.hpp"

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

//// test data for static visitor
static std::string _iteratorResult;

template <typename Part, typename Args>
class process_part {
public:
    auto operator() (Args&& args) const{
        _iteratorResult += Part(args).to_string();
        return 0;
    }
};
////


/// test data for partition -> tuple conversion
template <size_t ... index>
class processed_part_t {
public:
    template <typename Part>
    constexpr processed_part_t(Part&& part)
        :_data{ process(part.data(index)) ... }
    {

    }
    constexpr char process(const char c) const {
        return c == '1' ? '7' : c;
    }
    std::string to_string() const { return std::string({ _data[index] ... }); }
    constexpr char data(size_t i) const { return _data[i]; }


    template <typename Part>
    static constexpr auto get(Part&& p)
    {
        return make_processed_part(std::forward<Part>(p));
    }
private:
    const char _data[sizeof ... (index)];
};

class processor_class_t {
private:
    template <typename Part, size_t ... index>
    static constexpr auto make_processed_part_helper(Part&& part, std::index_sequence<index...>)
    {
        return processed_part_t<index...>(std::forward<Part>(part));
    }

    template <typename Part>
    static constexpr auto make_processed_part(Part&& part)
    {
        return make_processed_part_helper(std::forward<Part>(part), std::make_index_sequence<part.size()>());
    }
public:
    template <typename Part>
    static constexpr auto process(Part&& p)
    {
        return make_processed_part(std::forward<Part>(p));
    }
};
////

static void test_partition()
{
    constexpr auto part_length = 3;
    constexpr char test_data[] = "1234x5612x3456x1234x5678x";
    using parts = decltype(string_partition::make_partition<part_length>(test_data));
    partition_iterator::visit<parts, process_part>()(test_data);
    assert(_iteratorResult == std::string(test_data));
}

static void test_partition_transform()
{
    constexpr auto part_length = 3;
    constexpr char test_data[] = "1234x5612x3456x1234x5678x";
    using parts = decltype(string_partition::make_partition<part_length>(test_data));

    constexpr auto identity_tuple = partition_transform::convert<parts>(test_data);
    static_assert(identity_tuple.size() == 9, "");
    static_assert(tuple_utils::get<0>(identity_tuple).data(0) == '1', "");
    static_assert(tuple_utils::get<0>(identity_tuple).data(1) == '2', "");
    static_assert(tuple_utils::get<0>(identity_tuple).data(2) == '3', "");
    static_assert(tuple_utils::get<1>(identity_tuple).data(0) == '4', "");
    static_assert(tuple_utils::get<1>(identity_tuple).data(1) == 'x', "");
    static_assert(tuple_utils::get<1>(identity_tuple).data(2) == '5', "");
    static_assert(tuple_utils::get<2>(identity_tuple).data(0) == '6', "");
    static_assert(tuple_utils::get<2>(identity_tuple).data(1) == '1', "");
    static_assert(tuple_utils::get<2>(identity_tuple).data(2) == '2', "");

    constexpr auto transformed_tuple = partition_transform::convert<parts, processor_class_t>(test_data);
    static_assert(transformed_tuple.size() == 9, "");
    static_assert(tuple_utils::get<0>(transformed_tuple).data(0) == '7', "");
    static_assert(tuple_utils::get<0>(transformed_tuple).data(1) == '2', "");
    static_assert(tuple_utils::get<0>(transformed_tuple).data(2) == '3', "");
    static_assert(tuple_utils::get<1>(transformed_tuple).data(0) == '4', "");
    static_assert(tuple_utils::get<1>(transformed_tuple).data(1) == 'x', "");
    static_assert(tuple_utils::get<1>(transformed_tuple).data(2) == '5', "");
    static_assert(tuple_utils::get<2>(transformed_tuple).data(0) == '6', "");
    static_assert(tuple_utils::get<2>(transformed_tuple).data(1) == '7', "");
    static_assert(tuple_utils::get<2>(transformed_tuple).data(2) == '2', "");
    assert(tuple_utils::get<0>(transformed_tuple).to_string() == "723");
}

int main()
{
    test_type_list();
    test_string_parts();
    test_partition();
    test_partition_transform();
    return 0;
}
