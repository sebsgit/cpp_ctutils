#pragma once

#include <array>
#include <utility>

/// Constexpr utilities for std::array
namespace array_converter {
namespace priv {
template <typename Byte, size_t size, typename Char, size_t ... index_seq>
constexpr auto to_array_helper(const Char (&string)[size], std::index_sequence<index_seq...>)
{
    return std::array<Byte, size>{static_cast<Byte>(string[index_seq]) ...};
}

template <typename Byte,
    size_t size0,
    size_t size1,
    size_t ... seq_left,
    size_t ... seq_right,
    size_t actual_size0,
    size_t actual_size1
>
constexpr auto join_helper(const std::array<Byte, actual_size0>& left,
                           const std::array<Byte, actual_size1>& right,
                           std::index_sequence<seq_left...>,
                           std::index_sequence<seq_right...>)
{
    return std::array<Byte, size0 + size1>{left[seq_left] ..., right[seq_right] ...};
}

template <typename Byte, size_t size, size_t ... seq>
constexpr bool equal_helper(const std::array<Byte, size>& left, const std::array<Byte, size>& right, std::index_sequence<seq...>)
{
    return ((left[seq] == right[seq]) && ...);
}

}

template <typename Byte, typename Char, size_t size>
constexpr std::array<Byte, size> to_array(const Char (&string)[size]) {
    return priv::to_array_helper<Byte, size>(string, std::make_index_sequence<size>());
}

template <typename Byte, size_t size0, size_t size1>
constexpr auto join(const std::array<Byte, size0>& left, const std::array<Byte, size1>& right)
{
    return priv::join_helper<Byte, size0, size1>(left, right, std::make_index_sequence<size0>(), std::make_index_sequence<size1>());
}

template <typename Byte, size_t size0, size_t size1>
constexpr auto is_equal(const std::array<Byte, size0>& left, const std::array<Byte, size1>& right)
{
    return size0 == size1 && priv::equal_helper(left, right, std::make_index_sequence<size0>());
}

}
