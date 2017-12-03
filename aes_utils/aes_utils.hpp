#pragma once

#include <cinttypes>
#include <cstdlib>
#include <cstddef>
#include <utility>
#include <ostream>
#include <iterator>
#include <array>

#include "string_partition/array_converter.hpp"

/**
    Namespace contains utilities used in AES encryption algorithm.
*/
namespace aes_utils {
/**
    Compile time Rijndael s-box (forward and inverse).
*/
class s_box {
public:
    /// @return s-box value for a given byte
    static constexpr uint8_t value(const uint8_t b) { return _rijndael_box[b]; }
    /// @return inverse s-box value for a given byte
    static constexpr uint8_t inverse(const uint8_t b)
    {
        return _rijndael_box_inverse[b];
    }
    static constexpr uint8_t lookup_L(const uint8_t b) noexcept
    {
        return L_table[b];
    }
    static constexpr uint8_t multiply_galois(uint8_t b1, uint8_t b2){
        uint16_t res = L_table[b1] + L_table[b2];
        if( b1 == 0x01 ) return b2;
        if( b2 == 0x01 ) return b1;
        if( b1 == 0x00 || b2 == 0x00 ) return 0x00;
        if( res > 0xFF ){
            res -= 0xFF;
        }
        return E_table[static_cast<uint8_t>(res)];
    }

private:
    static inline constexpr uint8_t _rijndael_box[] = {
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B,
        0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
        0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26,
        0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
        0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2,
        0xEB, 0x27, 0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
        0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED,
        0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
        0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F,
        0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
        0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2, 0xCD, 0x0C, 0x13, 0xEC,
        0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
        0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14,
        0xDE, 0x5E, 0x0B, 0xDB, 0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
        0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D,
        0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
        0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F,
        0x4B, 0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
        0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98, 0x11,
        0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
        0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F,
        0xB0, 0x54, 0xBB, 0x16
    };

    static inline constexpr uint8_t _rijndael_box_inverse[] = {
        0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E,
        0x81, 0xF3, 0xD7, 0xFB, 0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87,
        0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB, 0x54, 0x7B, 0x94, 0x32,
        0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
        0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49,
        0x6D, 0x8B, 0xD1, 0x25, 0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16,
        0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92, 0x6C, 0x70, 0x48, 0x50,
        0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
        0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05,
        0xB8, 0xB3, 0x45, 0x06, 0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02,
        0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B, 0x3A, 0x91, 0x11, 0x41,
        0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
        0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8,
        0x1C, 0x75, 0xDF, 0x6E, 0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89,
        0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B, 0xFC, 0x56, 0x3E, 0x4B,
        0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
        0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59,
        0x27, 0x80, 0xEC, 0x5F, 0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D,
        0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF, 0xA0, 0xE0, 0x3B, 0x4D,
        0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
        0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63,
        0x55, 0x21, 0x0C, 0x7D
    };

    static inline constexpr uint8_t L_table[] = {
        0x00, 0x00, 0x19, 0x01, 0x32, 0x02, 0x1A, 0xC6, 0x4B, 0xC7, 0x1B, 0x68, 0x33, 0xEE, 0xDF, 0x03,
        0x64, 0x04, 0xE0, 0x0E, 0x34, 0x8D, 0x81, 0xEF, 0x4C, 0x71, 0x08, 0xC8, 0xF8, 0x69, 0x1C, 0xC1,
        0x7D, 0xC2, 0x1D, 0xB5, 0xF9, 0xB9, 0x27, 0x6A, 0x4D, 0xE4, 0xA6, 0x72, 0x9A, 0xC9, 0x09, 0x78,
        0x65, 0x2F, 0x8A, 0x05, 0x21, 0x0F, 0xE1, 0x24, 0x12, 0xF0, 0x82, 0x45, 0x35, 0x93, 0xDA, 0x8E,
        0x96, 0x8F, 0xDB, 0xBD, 0x36, 0xD0, 0xCE, 0x94, 0x13, 0x5C, 0xD2, 0xF1, 0x40, 0x46, 0x83, 0x38,
        0x66, 0xDD, 0xFD, 0x30, 0xBF, 0x06, 0x8B, 0x62, 0xB3, 0x25, 0xE2, 0x98, 0x22, 0x88, 0x91, 0x10,
        0x7E, 0x6E, 0x48, 0xC3, 0xA3, 0xB6, 0x1E, 0x42, 0x3A, 0x6B, 0x28, 0x54, 0xFA, 0x85, 0x3D, 0xBA,
        0x2B, 0x79, 0x0A, 0x15, 0x9B, 0x9F, 0x5E, 0xCA, 0x4E, 0xD4, 0xAC, 0xE5, 0xF3, 0x73, 0xA7, 0x57,
        0xAF, 0x58, 0xA8, 0x50, 0xF4, 0xEA, 0xD6, 0x74, 0x4F, 0xAE, 0xE9, 0xD5, 0xE7, 0xE6, 0xAD, 0xE8,
        0x2C, 0xD7, 0x75, 0x7A, 0xEB, 0x16, 0x0B, 0xF5, 0x59, 0xCB, 0x5F, 0xB0, 0x9C, 0xA9, 0x51, 0xA0,
        0x7F, 0x0C, 0xF6, 0x6F, 0x17, 0xC4, 0x49, 0xEC, 0xD8, 0x43, 0x1F, 0x2D, 0xA4, 0x76, 0x7B, 0xB7,
        0xCC, 0xBB, 0x3E, 0x5A, 0xFB, 0x60, 0xB1, 0x86, 0x3B, 0x52, 0xA1, 0x6C, 0xAA, 0x55, 0x29, 0x9D,
        0x97, 0xB2, 0x87, 0x90, 0x61, 0xBE, 0xDC, 0xFC, 0xBC, 0x95, 0xCF, 0xCD, 0x37, 0x3F, 0x5B, 0xD1,
        0x53, 0x39, 0x84, 0x3C, 0x41, 0xA2, 0x6D, 0x47, 0x14, 0x2A, 0x9E, 0x5D, 0x56, 0xF2, 0xD3, 0xAB,
        0x44, 0x11, 0x92, 0xD9, 0x23, 0x20, 0x2E, 0x89, 0xB4, 0x7C, 0xB8, 0x26, 0x77, 0x99, 0xE3, 0xA5,
        0x67, 0x4A, 0xED, 0xDE, 0xC5, 0x31, 0xFE, 0x18, 0x0D, 0x63, 0x8C, 0x80, 0xC0, 0xF7, 0x70, 0x07
    };

    static inline constexpr uint8_t E_table[] = {
        0x01, 0x03, 0x05, 0x0F, 0x11, 0x33, 0x55, 0xFF, 0x1A, 0x2E, 0x72, 0x96, 0xA1, 0xF8, 0x13, 0x35,
        0x5F, 0xE1, 0x38, 0x48, 0xD8, 0x73, 0x95, 0xA4, 0xF7, 0x02, 0x06, 0x0A, 0x1E, 0x22, 0x66, 0xAA,
        0xE5, 0x34, 0x5C, 0xE4, 0x37, 0x59, 0xEB, 0x26, 0x6A, 0xBE, 0xD9, 0x70, 0x90, 0xAB, 0xE6, 0x31,
        0x53, 0xF5, 0x04, 0x0C, 0x14, 0x3C, 0x44, 0xCC, 0x4F, 0xD1, 0x68, 0xB8, 0xD3, 0x6E, 0xB2, 0xCD,
        0x4C, 0xD4, 0x67, 0xA9, 0xE0, 0x3B, 0x4D, 0xD7, 0x62, 0xA6, 0xF1, 0x08, 0x18, 0x28, 0x78, 0x88,
        0x83, 0x9E, 0xB9, 0xD0, 0x6B, 0xBD, 0xDC, 0x7F, 0x81, 0x98, 0xB3, 0xCE, 0x49, 0xDB, 0x76, 0x9A,
        0xB5, 0xC4, 0x57, 0xF9, 0x10, 0x30, 0x50, 0xF0, 0x0B, 0x1D, 0x27, 0x69, 0xBB, 0xD6, 0x61, 0xA3,
        0xFE, 0x19, 0x2B, 0x7D, 0x87, 0x92, 0xAD, 0xEC, 0x2F, 0x71, 0x93, 0xAE, 0xE9, 0x20, 0x60, 0xA0,
        0xFB, 0x16, 0x3A, 0x4E, 0xD2, 0x6D, 0xB7, 0xC2, 0x5D, 0xE7, 0x32, 0x56, 0xFA, 0x15, 0x3F, 0x41,
        0xC3, 0x5E, 0xE2, 0x3D, 0x47, 0xC9, 0x40, 0xC0, 0x5B, 0xED, 0x2C, 0x74, 0x9C, 0xBF, 0xDA, 0x75,
        0x9F, 0xBA, 0xD5, 0x64, 0xAC, 0xEF, 0x2A, 0x7E, 0x82, 0x9D, 0xBC, 0xDF, 0x7A, 0x8E, 0x89, 0x80,
        0x9B, 0xB6, 0xC1, 0x58, 0xE8, 0x23, 0x65, 0xAF, 0xEA, 0x25, 0x6F, 0xB1, 0xC8, 0x43, 0xC5, 0x54,
        0xFC, 0x1F, 0x21, 0x63, 0xA5, 0xF4, 0x07, 0x09, 0x1B, 0x2D, 0x77, 0x99, 0xB0, 0xCB, 0x46, 0xCA,
        0x45, 0xCF, 0x4A, 0xDE, 0x79, 0x8B, 0x86, 0x91, 0xA8, 0xE3, 0x3E, 0x42, 0xC6, 0x51, 0xF3, 0x0E,
        0x12, 0x36, 0x5A, 0xEE, 0x29, 0x7B, 0x8D, 0x8C, 0x8F, 0x8A, 0x85, 0x94, 0xA7, 0xF2, 0x0D, 0x17,
        0x39, 0x4B, 0xDD, 0x7C, 0x84, 0x97, 0xA2, 0xFD, 0x1C, 0x24, 0x6C, 0xB4, 0xC7, 0x52, 0xF6, 0x01
    };
};

class rcon_table {
public:
    static constexpr auto at(size_t index) {
        return _rcon_data[index];
    }
private:
    static inline constexpr uint8_t _rcon_data[16] = {
        0x00, 0x01, 0x02, 0x04,
        0x08, 0x10, 0x20, 0x40,
        0x80, 0x1B, 0x36, 0x6C,
        0xD8, 0xAB, 0x4D, 0x9A
    };
};

//TODO: move to internal helper namespace
template <size_t size>
class byte_pack {
    static_assert(size == 4 || size == 16 || size == 20);
private:
    template <size_t ... index>
    constexpr byte_pack x_or_helper(const byte_pack& other, std::index_sequence<index...>) const noexcept {
        return byte_pack{ _data[index] ^ other._data[index] ... };
    }
    template <size_t ... index_seq>
    constexpr byte_pack set_helper(size_t index, uint8_t value, std::index_sequence<index_seq...>) const
    {
        return byte_pack{ index_seq == index ? value : _data[index_seq] ...  };
    }
    template <typename ... Boolean>
    static constexpr bool all_of(Boolean ... b)
    {
        return (static_cast<bool>(b) && ...);
    }
    template <size_t ... index_seq>
    static constexpr bool equal_helper(const byte_pack& left, const byte_pack& right, std::index_sequence<index_seq...>)
    {
        return all_of(left[index_seq] == right[index_seq] ...);
    }
    template <size_t output_size, size_t ... index_seq>
    static constexpr auto to_array_helper(const std::array<uint8_t, size>& data, std::index_sequence<index_seq...>)
    {
        return std::array<uint8_t, output_size>{data[index_seq] ...};
    }
public:
    template <typename ... Args>
    constexpr byte_pack(Args ... args)
        :_data{ static_cast<uint8_t>(args) ... }
    {}
    constexpr byte_pack(const byte_pack& other) noexcept
        :_data(other._data)
    {}
    template <typename Byte, size_t input_size, size_t ... index_seq>
    static constexpr byte_pack from_array(const std::array<Byte, input_size>& input, std::index_sequence<index_seq...>, size_t offset = 0)
    {
        return byte_pack(static_cast<uint8_t>(input[index_seq + offset]) ...);
    }
    constexpr auto operator[](size_t index) const noexcept {
        return _data[index];
    }
    constexpr auto to_array() const noexcept {
        return this->_data;
    }
    template <size_t output_size>
    constexpr auto to_array() const noexcept {
        static_assert(output_size <= size, "");
        return to_array_helper<output_size>(this->_data, std::make_index_sequence<output_size>());
    }
    constexpr byte_pack set(size_t index, uint8_t value) const
    {
        return set_helper(index, value, std::make_index_sequence<size>());
    }
    constexpr auto operator ()(size_t row_number, size_t column_number) const noexcept
    {
        return _data[row_number + 4 * column_number];
    }
    constexpr byte_pack set(size_t row_number, size_t column_number, uint8_t value) const
    {
        return this->set(row_number + 4 * column_number, value);
    }
    constexpr byte_pack x_or(const byte_pack& other) const noexcept {
        return x_or_helper(other, std::make_index_sequence<size>());
    }
    constexpr byte_pack shift_row_left(size_t row_number) const
    {
        return set(row_number, 0, (*this)(row_number, 1))
                .set(row_number, 1, (*this)(row_number, 2))
                .set(row_number, 2, (*this)(row_number, 3))
                .set(row_number, 3, (*this)(row_number, 0));
    }
    template <size_t count>
    constexpr byte_pack shift_row_left(size_t row_number) const
    {
        if constexpr (count == 0) {
            (void)row_number;
            return *this;
        } else {
            const auto tmp = this->shift_row_left(row_number);
            return tmp.template shift_row_left<count - 1>(row_number);
        }
    }
    friend constexpr bool operator== (const byte_pack& left, const byte_pack& right)
    {
        return equal_helper(left, right, std::make_index_sequence<size>());
    }
    friend constexpr bool operator != (const byte_pack& left, const byte_pack& right)
    {
        return !(left == right);
    }
    friend std::ostream& operator << (std::ostream& out, const byte_pack& data)
    {
        out << '{' << std::hex;
        for (auto c : data._data)
            out << static_cast<int>(c) << ' ';
        out << std::dec << '}';
        return out;
    }
private:
    const std::array<uint8_t, size> _data;
};

// TODO: docs
using word = byte_pack<4>;
using quad_word = byte_pack<16>;

template <size_t key_length>
class aes_key {
private:
    static_assert(key_length == 128 || key_length == 192 || key_length == 256,
        "key length must be 128, 192 or 256");

    static constexpr size_t get_data_size()
    {
        if constexpr (key_length == 128)
            return 176;
        else if constexpr (key_length == 192)
            return 208;
        else
            return 240;
    }

    static constexpr word rotate_word(const word& data, uint8_t rcon_index) {
        return word(s_box::value(data[1]) ^ rcon_table::at(rcon_index),
                s_box::value(data[2]),
                s_box::value(data[3]),
                s_box::value(data[0]));
    }

    template <size_t length, typename CharType, size_t ... index_seq>
    explicit constexpr aes_key(const CharType(&array)[length], std::index_sequence<index_seq...>)
        : _data{static_cast<unsigned char>(array[index_seq]) ...}
    {}
    template <size_t length, typename CharType, size_t byte_count, size_t ... prefix_seq, size_t ... byte_seq, size_t ... postfix_seq>
    explicit constexpr aes_key(const CharType(&array)[length], const byte_pack<byte_count>& bytes, std::index_sequence<prefix_seq...>, std::index_sequence<byte_seq ...>, std::index_sequence<postfix_seq...>)
        : _data{ static_cast<unsigned char>(array[prefix_seq]) ..., bytes[byte_seq] ..., array[byte_count + sizeof ... (prefix_seq) + postfix_seq] ... }
    {}
    //TODO: hard-coded to 176
    template <size_t p, size_t i>
    static constexpr aes_key expand_helper(const aes_key& key) {
        static_assert(key_length == 128, "implement missing key expand methods");
        if constexpr (p == 176)
            return key;
        else {
            const word b = (p % 16 == 0) ? rotate_word(key.get_word(p - 4), i) : key.get_word(p - 4);
            const auto new_key = key.set_word<p>(key.get_word(p - 16).x_or(b));
            return expand_helper<p + 4, (p % 16 == 0) ? i + 1 : i>(new_key);
        }
    }
public:
    static constexpr size_t key_size = key_length;
    static constexpr size_t data_size = get_data_size();

    template <size_t length, typename CharType>
    static constexpr auto create(const CharType(&array)[length]) {
        return aes_key<key_size>(array, std::make_index_sequence<length>());
    }

    constexpr word get_word(size_t offset) const
    {
        return word(_data[offset], _data[offset + 1], _data[offset + 2], _data[offset + 3]);
    }
    constexpr quad_word get_q_word(size_t offset) const
    {
        return quad_word(_data[offset], _data[offset + 1], _data[offset + 2], _data[offset + 3],
                _data[offset + 4], _data[offset + 5], _data[offset + 6], _data[offset + 7],
                _data[offset + 8], _data[offset + 9], _data[offset + 10], _data[offset + 11],
                _data[offset + 12], _data[offset + 13], _data[offset + 14], _data[offset + 15]);
    }
    template <size_t offset>
    constexpr aes_key set_word(const word& bytes) const {
        return aes_key<key_size>(_data, bytes, std::make_index_sequence<offset>(), std::make_index_sequence<4>(), std::make_index_sequence<data_size - 4 - offset>());
    }
    template <size_t offset>
    constexpr aes_key set_q_word(const quad_word& bytes) const
    {
        return aes_key<key_size>(_data, bytes, std::make_index_sequence<offset>(), std::make_index_sequence<16>(), std::make_index_sequence<data_size - 16 - offset>());
    }

    constexpr auto expand() const
    {
        return expand_helper<16, 1>(*this);
    }
    constexpr auto begin() const {
        return std::begin(_data);
    }
    constexpr auto end() const {
        return std::end(_data);
    }

    friend std::ostream& operator << (std::ostream& out, const aes_key& key)
    {
        out << '{' << std::hex;
        for (auto c : key._data)
            out << static_cast<int>(c) << ' ';
        out << std::dec << '}';
        return out;
    }

private:
    const unsigned char _data[data_size] = 0;
};

template <size_t key_length>
class aes_context {
private:
    static constexpr size_t number_of_rounds() {
        //TODO: hard-coded to aes-128
        return 10;
    }
    template <size_t ... index_seq>
    static constexpr quad_word substitute_helper(const quad_word& data, std::index_sequence<index_seq...>) noexcept
    {
        return quad_word(s_box::value(data[index_seq]) ...);
    }
    static constexpr uint8_t col_mix_helper(const quad_word& data, size_t column, uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    {
        return s_box::multiply_galois(a, data(0, column))
                ^ s_box::multiply_galois(b, data(1, column))
                ^ s_box::multiply_galois(c, data(2, column))
                ^ s_box::multiply_galois(d, data(3, column));
    }
    template <size_t index>
    constexpr quad_word encrypt_loop(const quad_word& data) const
    {
        return add_round_key<index>(column_mix(row_shift(s_box_replace(data))));
    }
    template <size_t round, size_t max_rounds>
    constexpr quad_word encrypt_helper(const quad_word& data) const
    {
        if constexpr (round == 0)
            return encrypt_helper<1, max_rounds>(add_round_key<0>(data));
        else if constexpr (round < max_rounds)
            return encrypt_helper<round + 1, max_rounds>(encrypt_loop<round>(data));
        else
            return add_round_key<round>(row_shift(s_box_replace(data)));
    }
    static constexpr quad_word s_box_replace(const quad_word& data) noexcept
    {
        return substitute_helper(data, std::make_index_sequence<16>());
    }
    static constexpr quad_word column_mix(const quad_word& data, size_t column) noexcept
    {
        return data.set(0, column, col_mix_helper(data, column, 0x2, 0x3, 0x1, 0x1))
            .set(1, column, col_mix_helper(data, column, 0x1, 0x2, 0x3, 0x1))
            .set(2, column, col_mix_helper(data, column, 0x1, 0x1, 0x2, 0x3))
            .set(3, column, col_mix_helper(data, column, 0x3, 0x1, 0x1, 0x2));
    }
    static constexpr quad_word column_mix(const quad_word &data) noexcept
    {
        return column_mix(column_mix(column_mix(column_mix(data, 0), 1), 2), 3);
    }
    static constexpr quad_word row_shift(const quad_word& data) noexcept
    {
        return data.shift_row_left<1>(1).shift_row_left<2>(2).shift_row_left<3>(3);
    }
    template <size_t round_number>
    constexpr quad_word add_round_key(const quad_word& data) const
    {
        return data.x_or(this->_key.get_q_word(round_number * 16));
    }
    template <typename Byte, size_t size, size_t array_size, size_t offset = 0>
    constexpr auto encrypt_array_helper(const std::array<Byte, array_size>& data) const
    {
        if constexpr (size <= 16) {
            return this->encrypt(quad_word::from_array(data, std::make_index_sequence<size>(), offset)).template to_array<size>();
        } else {
            return array_converter::join( this->encrypt(quad_word::from_array(data, std::make_index_sequence<16>(), offset)).to_array(),
                                          this->encrypt_array_helper<Byte, size - 16, array_size, offset + 16>(data) );
        }
    }
public:
    constexpr aes_context(const aes_key<key_length>& key)
        :_key(key)
    {}
    constexpr quad_word encrypt(const quad_word& data) const
    {
        return encrypt_helper<0, number_of_rounds()>(data);
    }
    template <typename Byte, size_t array_size>
    constexpr auto encrypt(const std::array<Byte, array_size>& data) const
    {
        return this->encrypt_array_helper<Byte, array_size, array_size, 0>(data);
    }

private:
    const aes_key<key_length> _key;
};
}
