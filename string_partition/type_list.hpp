#pragma once

#include <cstddef>

/**
    A simple type list implementation.
*/
namespace type_list {

namespace priv {
class sentinel{ };

template <typename T>
class node {
public:
    using type = T;
    using rest = priv::sentinel;
};

template <typename Head, typename Tail>
class list {
public:
    using type = Head;
    using rest = Tail;
};
}

template <typename T>
class create
{
public:
    using type = priv::node<T>;
    using rest = priv::sentinel;
};

template <typename List, typename Type>
class append {
public:
    using type = priv::list<Type, List>;
    using rest = priv::sentinel;
};

template <typename List>
class count;

template <>
class count<priv::sentinel>
{
public:
  static constexpr size_t size = 0;
};

template <typename T>
class count {
public:
    static constexpr size_t size = 1 + count<typename T::rest>::size;
};

namespace priv {
template <typename List, size_t index>
class at_reversed;

template <typename List>
class at_reversed<List, 0>
{
public:
    using type = typename List::type;
};

template <typename List, size_t index>
class at_reversed
{
public:
    using type = typename at_reversed<typename List::rest, index - 1>::type;
};
}

template <typename List, size_t index>
class at
{
public:
    static_assert(index < count<List>::size, "type list index out of range");
    using type = typename priv::at_reversed<List, count<List>::size - index - 1>::type;
};

}
