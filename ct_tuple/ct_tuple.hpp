#pragma once

#include <cstddef>
#include <utility>

/**
    Compile time tuple class, with a constexpr `get` operator.
*/
namespace tuple_utils {
template <typename ... Types>
class ct_tuple;

template <typename T>
class ct_tuple<T> {
public:
    constexpr ct_tuple(const T& t) : _t(t)
    { }
    constexpr T get() const { return _t; }

    template <typename K>
    constexpr auto add(K && k) const { return ct_tuple<T, K>(_t, std::forward<K>(k)); }

    static constexpr size_t size() { return 1; }
private:
    const T _t;
};

template <typename First, typename ... Rest>
class ct_tuple<First, Rest...> {
public:
    constexpr ct_tuple(const First& first, const Rest & ... rest)
        : _head(first)
        , _tail(rest...)
    {

    }

    constexpr ct_tuple(const First& first, const ct_tuple<Rest...> & rest)
        : _head(first)
        , _tail(rest)
    {

    }

    constexpr auto get() const { return _head; }
    constexpr auto tail() const { return _tail; }

    template <typename K>
    constexpr auto add(K&& k) const
    {
        return ct_tuple<First, Rest..., K>(_head, _tail.add(std::forward<K>(k)));
    }

    static constexpr size_t size() { return 1 + ct_tuple<Rest...>::size(); }

private:
    const First _head;
    const ct_tuple<Rest...> _tail;
};

namespace priv {
/**
    Helper class for the @ref get function.
*/
template <typename Tuple, size_t counter>
class getter;

template <typename Tuple>
class getter<Tuple, 0>
{
public:
    static constexpr auto get(const Tuple& t) { return t.get(); }
};

template <typename Tuple, size_t size>
class getter
{
public:
    static constexpr auto get(const Tuple& t) { return getter<decltype(t.tail()), size - 1>::get(t.tail()); }
};
}

/**
    Returns the element located at given position in the tuple.
    @tparam index Position to query.
    @tparam Tuple Type of the tuple object.
    @param t Tuple object to query.
    @return Value at @ref index.
*/
template <size_t index, typename Tuple>
constexpr auto get(const Tuple& t)
{
    static_assert(index < Tuple::size(), "index out of tuple bounds");
    return priv::getter<Tuple, index>::get(t);
}

template <typename T>
constexpr auto make_tuple(T&& t)
{
    return ct_tuple<T>(std::forward<T>(t));
}
}
