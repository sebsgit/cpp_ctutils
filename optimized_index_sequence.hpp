#pragma once

#include <utility>

namespace sequence_generator {

template <size_t ... seq, size_t ... seq2>
constexpr auto make_joined(std::index_sequence<seq...>, std::index_sequence<seq2...>) noexcept {
    return std::index_sequence<seq..., seq2...>();
}

template <size_t offset, size_t ... seq>
constexpr auto offset_sequence(std::index_sequence<seq...>) noexcept {
    return std::index_sequence<offset + seq ...>();
};

template <size_t Start, size_t Length, bool ShortSequence = (Length < 8)>
struct generator;

template <size_t Start, size_t Length>
struct generator<Start, Length, true> {
    using type = decltype(offset_sequence<Start>(std::make_index_sequence<Length>()));
};

template <size_t Start, size_t Length>
struct generator<Start, Length, false> {
    using seq1 = typename generator<Start, Length / 2>::type;
    using seq2 = typename generator<Start + Length / 2, Length - (Length / 2)>::type;
    using type = decltype (make_joined(seq1(), seq2()));
};

template <size_t Start, size_t Length>
constexpr auto make_index_sequence() noexcept {
    return typename generator<Start, Length>::type();
};

template <size_t N>
constexpr auto make_index_sequence() noexcept {
    return make_index_sequence<0, N>();
};

} // namespace
