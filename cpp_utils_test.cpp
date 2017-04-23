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
    constexpr processed_part_t(const Part& part)
        :_data{ process(part.data(index)) ... }
    {

    }
    static constexpr char process(const char c) {
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

/**
    /// @TODO structure the tests somehow, cmake maybe
*/

static void test_sha1_internal_utils()
{
    constexpr auto ctx = sha1_utils::sha1_create_context("abcd");
    assert(ctx.buffer_length == 4);
    static_assert(ctx.buffer[0] == 'a', "");
    static_assert(ctx.buffer[1] == 'b', "");
    static_assert(ctx.buffer[2] == 'c', "");
    static_assert(ctx.buffer[3] == 'd', "");
    static_assert(ctx.buffer[4] == '\0', "");
    constexpr auto ctx_copied = ctx.append("uXYZy", 1, 3);
    assert(ctx_copied.buffer_length == 7);
    static_assert(ctx_copied.buffer[0] == 'a', "");
    static_assert(ctx_copied.buffer[1] == 'b', "");
    static_assert(ctx_copied.buffer[2] == 'c', "");
    static_assert(ctx_copied.buffer[3] == 'd', "");
    static_assert(ctx_copied.buffer[4] == 'X', "");
    static_assert(ctx_copied.buffer[5] == 'Y', "");
    static_assert(ctx_copied.buffer[6] == 'Z', "");
    static_assert(ctx_copied.buffer[7] == '\0', "");
    assert(std::string((const char*)ctx_copied.buffer) == "abcdXYZ");
}

static void test_sha1_utils()
{
    test_sha1_internal_utils();

    constexpr auto ctx = sha1_utils::sha1_create_context("text to sha1");
    // verify intermediate data in sha1
    constexpr auto X = sha1_utils::priv::sha1_compute().add_context_round(ctx).add_rotate_round();
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
    constexpr auto ctx_updated = sha1_utils::priv::sha1_calc(ctx);
    static_assert(ctx_updated.result[0] == 1921304193, "");
    static_assert(ctx_updated.result[1] == 2414806865, "");
    static_assert(ctx_updated.result[2] == 1370285663, "");
    static_assert(ctx_updated.result[3] == 895243306, "");
    static_assert(ctx_updated.result[4] == 3663183577, "");

    // after adding more than 64 characters the buffer should wrap and update
    // the result for the first 64 byte batch
    constexpr auto context = sha1_utils::sha1_context();
    constexpr auto context_appended = sha1_utils::sha1_add_data(context, "E this text has exactly 67 letters, so it's easy to test some code.");
    static_assert(context_appended.buffer[0] == 'd', "");
    static_assert(context_appended.buffer[1] == 'e', "");
    static_assert(context_appended.buffer[2] == '.', "");
    static_assert(context_appended.buffer[3] == '\0', "");
    assert(context_appended.buffer_length == 3);
    static_assert(context_appended.result[0] == 12576993, "");
    static_assert(context_appended.result[1] == 1063349476, "");
    static_assert(context_appended.result[2] == 1393575024, "");
    static_assert(context_appended.result[3] == 2380056992, "");
    static_assert(context_appended.result[4] == 554802719, "");
    static_assert(context_appended.padding_byte() == 53, "");
    static_assert(context_appended.message_length*8 == 536, "");
    constexpr auto lbuff = sha1_utils::priv::sha1_length_buffer(context_appended.message_length);
    static_assert(lbuff.data[0] == 0, "");
    static_assert(lbuff.data[1] == 0, "");
    static_assert(lbuff.data[2] == 0, "");
    static_assert(lbuff.data[3] == 0, "");
    static_assert(lbuff.data[4] == 0, "");
    static_assert(lbuff.data[5] == 0, "");
    static_assert(lbuff.data[6] == 2, "");
    static_assert(lbuff.data[7] == 24, "");

    constexpr auto ctx_final = sha1_utils::sha1_finalize(context_appended);
    static_assert(ctx_final.result[0] == 4222907567, "");
    static_assert(ctx_final.result[1] == 4172679190, "");
    static_assert(ctx_final.result[2] == 2502518017, "");
    static_assert(ctx_final.result[3] == 133181236, "");
    static_assert(ctx_final.result[4] == 2662887834, "");

    {
        constexpr auto context_unpadded = sha1_utils::sha1_add_data(context, "E this text has exactly 62 letters, so it's easy to test some.");
        constexpr auto ctx_final = sha1_utils::sha1_finalize(context_unpadded);
        static_assert(ctx_final.result[0] == 195749063, "");
        static_assert(ctx_final.result[1] == 2775979648, "");
        static_assert(ctx_final.result[2] == 527287973, "");
        static_assert(ctx_final.result[3] == 1191620250, "");
        static_assert(ctx_final.result[4] == 300289637, "");
        constexpr auto sha1_result = sha1_utils::sha1_result(ctx_final);
        static_assert(sha1_result[0] == 0xb, "");
        static_assert(sha1_result[1] == 0xaa, "");
        static_assert(sha1_result[2] == 0xe4, "");
        static_assert(sha1_result[3] == 0xc7, "");
        static_assert(sha1_result[4] == 0xa5, "");
        static_assert(sha1_result[5] == 0x76, "");
        static_assert(sha1_result[6] == 0x16, "");
        static_assert(sha1_result[7] == 0x80, "");
        static_assert(sha1_result[8] == 0x1f, "");
        static_assert(sha1_result[9] == 0x6d, "");
        static_assert(sha1_result[10] == 0xc6, "");
        static_assert(sha1_result[11] == 0xa5, "");
        static_assert(sha1_result[12] == 0x47, "");
        static_assert(sha1_result[13] == 0x06, "");
        static_assert(sha1_result[14] == 0xae, "");
        static_assert(sha1_result[15] == 0x9a, "");
        static_assert(sha1_result[16] == 0x11, "");
        static_assert(sha1_result[17] == 0xe6, "");
        static_assert(sha1_result[18] == 0x0e, "");
        static_assert(sha1_result[19] == 0x65, "");
    }
    {
        constexpr auto hash = sha1_utils::sha1("maybe not the best text to test the sha1");
        // b6 7b 33 7c 3c f5 45 3e 9d 29 1f 16 01 70 91 e8 96 e4 9c b5
        static_assert(hash[0] == 0xb6, "");
        static_assert(hash[1] == 0x7b, "");
        static_assert(hash[2] == 0x33, "");
        static_assert(hash[3] == 0x7c, "");
        static_assert(hash[4] == 0x3c, "");
        static_assert(hash[5] == 0xf5, "");
        static_assert(hash[6] == 0x45, "");
        static_assert(hash[7] == 0x3e, "");
        static_assert(hash[8] == 0x9d, "");
        static_assert(hash[9] == 0x29, "");
        static_assert(hash[10] == 0x1f, "");
        static_assert(hash[11] == 0x16, "");
        static_assert(hash[12] == 0x01, "");
        static_assert(hash[13] == 0x70, "");
        static_assert(hash[14] == 0x91, "");
        static_assert(hash[15] == 0xe8, "");
        static_assert(hash[16] == 0x96, "");
        static_assert(hash[17] == 0xe4, "");
        static_assert(hash[18] == 0x9c, "");
        static_assert(hash[19] == 0xb5, "");
    }
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
