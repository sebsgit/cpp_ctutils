#pragma once

#include "type_list.hpp"
#include "../ct_tuple/ct_tuple.hpp"

/**
    Utilities for converting the string partition into a constexpr tuple object.
*/
namespace partition_transform {
/// identity transform, returns the unchanged partition element
class identity_transformer {
public:
    template <typename Part>
    static constexpr auto process(Part&& p)
    {
        return std::forward<Part>(p);
    }
};

namespace priv {
/**
    String partition converter template.
    @tparam List String partition to convert.
    @tparam Proc Optional processor class, should expose a static `process` template method. This method will be called for each partition element.
    @tparam count Recursion guard.
*/
template <typename List, typename Proc, size_t count>
class list_to_tuple_t;

template <typename List, typename Proc>
class list_to_tuple_t<List, Proc, 0> {
public:
    template <typename Args>
    static constexpr auto convert(Args&& args)
    {
        using type = typename type_list::at<List, 0>::type;
        return tuple_utils::make_tuple(Proc::process(type(args)));
    }
};

template <typename List, typename Proc, size_t count>
class list_to_tuple_t {
public:
    template <typename Args>
    static constexpr auto convert(Args&& args)
    {
        using type = typename type_list::at<List, count>::type;
        using next_type = list_to_tuple_t<List, Proc, count - 1>;
        return next_type::convert(args).add(Proc::process(type(args)));
    }
};
}

/**
    Method for converting a string partition into a constexpr tuple object.
    @tparam List String partitioning to convert.
    @tparam Proc Optional processor class, should expose a static `process` template method. This method will be called for each partition element.
    @tparam Args Deduced argument type.
    @param args Argument passed to each partition element.
*/
template <typename List, typename Proc = identity_transformer, typename Args>
constexpr auto convert(Args&& args)
{
    return priv::list_to_tuple_t<List, Proc, type_list::count<List>::size - 1>::convert(args);
}
}
