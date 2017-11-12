#pragma once

#include "ct_tuple/ct_tuple.hpp"

namespace ctml {
/**
    Compile time generator of recursive integer sequences.
    @tparam Generator A generator class, should provide a static `next(int seq0, ..., int seqN)` method.
    @tparam initSequence Initial values for the sequence. Next values will be generated recursively from the previous.
*/
template <typename Generator, int ... initSequence>
class int_sequence_generator {
private:
    using Tuple = decltype(tuple_utils::make_tuple(initSequence ...));

    template <size_t ... index>
    static constexpr auto expand_tuple(const Tuple& t, std::index_sequence<index...>)
    {
        return t.add(Generator::next(t.get(index) ...)).remove_first();
    }

    static constexpr int at(int i, const Tuple& t)
    {
        return i == 0 ? t.get(0) : at(i - 1, expand_tuple(t, std::make_index_sequence<Tuple::size()>()));
    }
public:
    /**
        Retrieve the sequence value at a given index.
        @param i Index in the sequence to return the value for.
        @return Value at index i.
    */
    static constexpr auto at(int i)
    {
        return at(i, tuple_utils::make_tuple(initSequence ...));
    }
};
}
