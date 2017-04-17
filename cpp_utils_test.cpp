#include "string_partition/string_part.hpp"
#include "string_partition/type_list.hpp"
#include "string_partition/string_partition.hpp"
#include "string_partition/partition_iterator.hpp"
#include "string_partition/partition_converter.hpp"
#include "aes_utils/aes_utils.hpp"
#include "sha1/sha1_utils.hpp"

#include <iostream>
#include <typeinfo>
#include <cassert>

_AES_UTILS_IMPL_;

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

static void test_aes_utils()
{
    for (int i=0 ; i<256 ; ++i)
        assert(aes_utils::s_box::value(aes_utils::s_box::inverse(i)) == i);
    static_assert(aes_utils::s_box::value(aes_utils::s_box::inverse(0x4f)) == 0x4f, "");
    static_assert(aes_utils::s_box::value(0xc9) == 0xdd, "");
}

static void test_sha1_utils()
{
    constexpr auto ctx = sha1_utils::sha1_create_context("text to sha1");
    // verify intermediate data in sha1
    constexpr auto X = sha1_utils::sha1_compute().add_context_round(ctx).add_rotate_round();
    static_assert(X.x[0] == 1952807028, "0");
    static_assert(X.x[1] == 544501536, "1");
    static_assert(X.x[2] == 1936220465, "2");
    static_assert(X.x[5] == 0, "5");
    static_assert(X.x[11] == 0, "11");
    static_assert(X.x[15] == 0, "15");
    static_assert(X.x[19] == 473195796, "19");
    static_assert(X.x[20] == 2178006144, "20");
    static_assert(X.x[21] == 3449914565, "21");
    static_assert(X.x[55] == 3945288533, "55");
    static_assert(X.x[56] == 3789232617, "56");
    static_assert(X.x[57] == 1207579205, "57");
    static_assert(X.x[77] == 3351878526, "77");
    static_assert(X.x[78] == 3945703551, "78");
    static_assert(X.x[79] == 1796382940, "79");
    // main sha1 computation routine
    constexpr auto ctx_updated = sha1_utils::sha1_calc(ctx);
    static_assert(ctx_updated.result[0] == 1921304193, "");
    static_assert(ctx_updated.result[1] == 2414806865, "");
    static_assert(ctx_updated.result[2] == 1370285663, "");
    static_assert(ctx_updated.result[3] == 895243306, "");
    static_assert(ctx_updated.result[4] == 3663183577, "");
    //@todo finalization
}

int main()
{
    test_type_list();
    test_string_parts();
    test_partition();
    test_partition_transform();
    test_aes_utils();
    test_sha1_utils();
    return 0;
}
