#pragma once

#include <array>
#include <utility>

/// Constexpr utilities for std::array
namespace array_converter {
namespace priv {
template <typename Byte, size_t size>
class array_holder {
public:
    template <typename Char, size_t ... index_seq>
    explicit constexpr array_holder(const Char (&string)[size], std::index_sequence<index_seq...>)
        :_data{static_cast<Byte>(string[index_seq]) ...}
    {

    }
    constexpr auto data() const { return this->_data; }
private:
    const std::array<Byte, size> _data;
};

template <typename Byte, size_t size0, size_t size1>
class join_helper {
public:
    template <size_t ... seq_left, size_t ... seq_right, size_t actual_size0, size_t actual_size1>
    constexpr join_helper(const std::array<Byte, actual_size0>& left, const std::array<Byte, actual_size1>& right
                          , std::index_sequence<seq_left...>, std::index_sequence<seq_right...>)
        :_result{ left[seq_left] ..., right[seq_right] ... }
    {

    }
    constexpr auto result() const { return this->_result; }
private:
    const std::array<Byte, size0 + size1> _result;
};

}

template <typename Byte, typename Char, size_t size>
constexpr std::array<Byte, size> to_array(const Char (&string)[size]) {
    return priv::array_holder<Byte, size>(string, std::make_index_sequence<size>()).data();
}

template <typename Byte, size_t size0, size_t size1>
constexpr auto join(const std::array<Byte, size0>& left, const std::array<Byte, size1>& right)
{
    return priv::join_helper<Byte, size0, size1>(left, right, std::make_index_sequence<size0>(), std::make_index_sequence<size1>()).result();
}

}
