#include "ctml/ctml/int_sequence_generator.hpp"
#include "string_partition/partition_converter.hpp"
#include "string_partition/partition_iterator.hpp"
#include "string_partition/string_part.hpp"
#include "string_partition/string_partition.hpp"
#include "string_partition/type_list.hpp"
#include "string_partition/array_converter.hpp"
#if __cplusplus >= 201703L
#include "aes_utils/aes_utils.hpp"
#endif
#include "sha1/sha1_utils.hpp"

#include <cassert>
#include <iostream>
#include <typeinfo>
#include <algorithm>

static void test_type_list() {
  using list = type_list::create<void>::type;
  using add_int = type_list::append<list, int>::type;
  using add_double = type_list::append<add_int, double>::type;
  static_assert(type_list::count<add_double>::size == 3, "size");
  static_assert(std::is_same<void, type_list::at<add_double, 0>::type>::value,
                "list[0] == void");
  static_assert(std::is_same<int, type_list::at<add_double, 1>::type>::value,
                "list[1] == int");
  static_assert(std::is_same<double, type_list::at<add_double, 2>::type>::value,
                "list[2] == double");
}

static void test_string_parts() {
  constexpr auto filled_part =
      string_part::make_filed_part<16, 0, sizeof("two")>("two");
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

template <typename Part, typename Args> class process_part {
public:
  auto operator()(Args &&args) const {
    _iteratorResult += Part(args).to_string();
    return 0;
  }
};
////

/// test data for partition -> tuple conversion
template <size_t... index> class processed_part_t {
public:
  template <typename Part>
  constexpr processed_part_t(const Part &part)
      : _data{process(part.data(index))...} {}
  static constexpr char process(const char c) { return c == '1' ? '7' : c; }
  std::string to_string() const { return std::string({_data[index]...}); }
  constexpr char data(size_t i) const { return _data[i]; }

  template <typename Part> static constexpr auto get(Part &&p) {
    return make_processed_part(std::forward<Part>(p));
  }

private:
  const char _data[sizeof...(index)];
};

class processor_class_t {
private:
  template <typename Part, size_t... index>
  static constexpr auto
  make_processed_part_helper(Part &&part, std::index_sequence<index...>) {
    return processed_part_t<index...>(std::forward<Part>(part));
  }

  template <typename Part>
  static constexpr auto make_processed_part(Part &&part) {
    return make_processed_part_helper(std::forward<Part>(part),
                                      std::make_index_sequence<Part::size()>());
  }

public:
  template <typename Part> static constexpr auto process(Part &&p) {
    return make_processed_part(std::forward<Part>(p));
  }
};
////

static void test_partition() {
  constexpr auto part_length = 3;
  constexpr char test_data[] = "1234x5612x3456x1234x5678x";
  using parts =
      decltype(string_partition::make_partition<part_length>(test_data));
  partition_iterator::visit<parts, process_part>()(test_data);
  assert(_iteratorResult == std::string(test_data));
}

static void test_partition_transform() {
  constexpr auto part_length = 3;
  constexpr char test_data[] = "1234x5612x3456x1234x5678x";
  using parts =
      decltype(string_partition::make_partition<part_length>(test_data));

  constexpr auto identity_tuple =
      partition_transform::convert<parts>(test_data);
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

  constexpr auto transformed_tuple =
      partition_transform::convert<parts, processor_class_t>(test_data);
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

static void test_aes_utils() {
#if __cplusplus >= 201703L
  std::cout << "with AES\n";
  {
      constexpr aes_utils::byte_pack<4> bytes{0x1, 0x2, 0x4, 0x7};
      static_assert(bytes[0] == 0x1, "");
      static_assert(bytes[1] == 0x2, "");
      static_assert(bytes[2] == 0x4, "");
      static_assert(bytes[3] == 0x7, "");
      constexpr aes_utils::byte_pack<4> copied(bytes);
      static_assert(copied[0] == 0x1, "");
      static_assert(copied[1] == 0x2, "");
      static_assert(copied[2] == 0x4, "");
      static_assert(copied[3] == 0x7, "");

      constexpr auto x_or = bytes.x_or(copied);
      static_assert(x_or[0] == 0, "");
      static_assert(x_or[1] == 0, "");
      static_assert(x_or[2] == 0, "");
      static_assert(x_or[3] == 0, "");

      constexpr aes_utils::quad_word test_16{0x51, 0x59, 0x2, 0x7a, 0xf4, 0x81, 0x1, 0x0,
                                            0x66, 0x12, 0xba, 0xcd, 0x7d, 0x71, 0x22, 0x6};
      constexpr aes_utils::quad_word test_16_shifted_left{0xf4, 0x59, 0x2, 0x7a, 0x66, 0x81, 0x1, 0,
                                                          0x7d, 0x12, 0xba, 0xcd, 0x51, 0x71, 0x22, 0x6};
      constexpr auto shifted = test_16.shift_row_left(0);
      static_assert(shifted == test_16_shifted_left, "");
  }
  for (int i = 0; i < 256; ++i)
    assert(aes_utils::s_box::value(aes_utils::s_box::inverse(i)) == i);
  static_assert(
      aes_utils::s_box::value(aes_utils::s_box::inverse(0x4f)) == 0x4f, "");
  static_assert(aes_utils::s_box::value(0xc9) == 0xdd, "");
  static_assert(aes_utils::aes_key<128>::data_size == 176, "");
  constexpr auto key = aes_utils::aes_key<128>::create("test_key");
  constexpr auto word = key.get_word(1);
  static_assert(word[0] == 'e', "");
  static_assert(word[1] == 's', "");
  static_assert(word[2] == 't', "");
  static_assert(word[3] == '_', "");
  constexpr auto key_new = key.set_word<3>(aes_utils::word('x', 'y', 'z', 'w'));
  static_assert(key_new.get_word(0)[0] == 't', "");
  static_assert(key_new.get_word(0)[1] == 'e', "");
  static_assert(key_new.get_word(0)[2] == 's', "");
  static_assert(key_new.get_word(0)[3] == 'x', "");
  static_assert(key_new.get_word(4)[0] == 'y', "");
  static_assert(key_new.get_word(4)[1] == 'z', "");
  static_assert(key_new.get_word(4)[2] == 'w', "");
  static_assert(key_new.get_word(4)[3] == 'y', "");
  constexpr auto expanded_key = key_new.expand();
  const uint8_t expanded_key_data_start[] = {
    0x74, 0x65, 0x73, 0x78, 0x79, 0x7a, 0x77, 0x79,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x16 ,0x6, 0x10, 0x1b, 0x6f, 0x7c, 0x67,
    0x62, 0x6f, 0x7c, 0x67, 0x62, 0x6f, 0x7c, 0x67
  };
  const uint8_t expanded_key_data_end[] = {
     0x12, 0x96, 0x1e, 0x68, 0x6d, 0xa0, 0xe4,
     0x95, 0xee, 0x4f, 0x49, 0x2f, 0x6c, 0x44,
     0x50, 0x4a, 0x7e, 0xd2, 0x4e, 0x22, 0x13
  };
  assert(std::search(expanded_key.begin(), expanded_key.end(), std::begin(expanded_key_data_start), std::end(expanded_key_data_start)) == expanded_key.begin());
  assert(std::abs(std::distance(expanded_key.end(), std::search(expanded_key.begin(), expanded_key.end(), std::begin(expanded_key_data_end), std::end(expanded_key_data_end)))) == sizeof(expanded_key_data_end));

  constexpr aes_utils::quad_word test_16{0x51, 0x59, 0x2, 0x7a, 0xf4, 0x81, 0x1, 0x0,
                                        0x66, 0x12, 0xba, 0xcd, 0x7d, 0x71, 0x22, 0x6};
  constexpr aes_utils::quad_word encrypted_data{0xfa, 0xfa, 0xb8, 0xd8, 0x26, 0x3, 0xc4, 0x9b,
                                                0x15, 0x91, 0x2d, 0x88, 0x4d, 0xa8, 0x98, 0x92};
  constexpr aes_utils::aes_context<128> context = aes_utils::aes_context<128>(expanded_key);
  constexpr auto secret = context.encrypt(test_16);
  static_assert(secret != test_16, "");
  static_assert(secret == encrypted_data, "");

  {
  constexpr std::array<unsigned char, 32> plaintext2 = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34,
                                                         0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
  constexpr unsigned char key2[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
  constexpr std::array<unsigned char, 32> expected2 = { 0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32,
                                          0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32};

  constexpr auto key_test = aes_utils::aes_key<128>::create(key2);
  constexpr aes_utils::aes_context<128> ctx(key_test.expand());
  constexpr auto result = ctx.encrypt(plaintext2);
  static_assert(expected2[0] == result[0], "");
  static_assert(expected2[17] == result[17], "");
  }

#endif
}

/**
    /// @TODO structure the tests somehow, cmake maybe
*/

static void test_sha1_internal_utils() {
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
  assert(std::string((const char *)ctx_copied.buffer) == "abcdXYZ");
}

static void test_sha1_utils() {
  test_sha1_internal_utils();

  constexpr auto ctx = sha1_utils::sha1_create_context("text to sha1");
  // verify intermediate data in sha1
  constexpr auto X = sha1_utils::priv::sha1_compute()
                         .add_context_round(ctx)
                         .add_rotate_round();
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
  constexpr auto context_appended = sha1_utils::sha1_add_data(
      context,
      "E this text has exactly 67 letters, so it's easy to test some code.");
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
  static_assert(context_appended.message_length * 8 == 536, "");
  constexpr auto lbuff =
      sha1_utils::priv::sha1_length_buffer(context_appended.message_length);
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
    constexpr auto context_unpadded = sha1_utils::sha1_add_data(
        context,
        "E this text has exactly 62 letters, so it's easy to test some.");
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
    constexpr auto hash =
        sha1_utils::sha1("maybe not the best text to test the sha1");
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

static void test_tuple() {
  constexpr auto tuple = tuple_utils::make_tuple(1, 2, 3, 4);
  static_assert(tuple.size() == 4, "");
  static_assert(tuple.get(0) == 1, "");
  static_assert(tuple.get(1) == 2, "");
  static_assert(tuple.get(2) == 3, "");
  static_assert(tuple.get(3) == 4, "");
  constexpr auto tuple1 = tuple.add(10);
  static_assert(tuple1.size() == 5, "");
  static_assert(tuple1.get(4) == 10, "");
  constexpr auto tuple2 = tuple1.remove_first().remove_first();
  static_assert(tuple2.size() == 3, "");
  static_assert(tuple2.get(2) == 10, "");
}

static void test_int_sequence_generator() {

  class fibo {
  public:
    static constexpr int next(int a, int b) { return a + b; }
  };

  using gen = ctml::int_sequence_generator<fibo, 0, 1>;
  static_assert(gen::at(0) == 0, "");
  static_assert(gen::at(1) == 1, "");
  static_assert(gen::at(2) == 1, "");
  static_assert(gen::at(3) == 2, "");
  static_assert(gen::at(4) == 3, "");
  static_assert(gen::at(5) == 5, "");
  static_assert(gen::at(6) == 8, "");
  static_assert(gen::at(7) == 13, "");
  static_assert(gen::at(8) == 21, "");
  static_assert(gen::at(9) == 34, "");
  static_assert(gen::at(17) == 1597, "");
}

int main() {
  test_tuple();
  test_int_sequence_generator();
  test_type_list();
  test_string_parts();
  test_partition();
  test_partition_transform();
  test_aes_utils();
  test_sha1_utils();
  return 0;
}
