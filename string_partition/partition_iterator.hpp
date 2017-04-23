#pragma once

#include "string_partition.hpp"

/**
    Provides helper methods to iterate the compile time string partition.
*/
namespace partition_iterator {

namespace priv {
/**
    Applies a given visitor to a partition element at a given index.
    @tparam List String partition.
    @tparam Visitor Visitor template. New visitor object is instantiated for every partition element.
    @tparam Part Type of the visited partition element.
    @tparam Args Argument passed to the visitor (partitioned string).
    @tparam index Index in the partition to apply the visitor at.
*/
template <typename List, template<typename Part, typename Args> class Visitor, size_t index>
class visit_at {
public:
    /**
        Visitor application operator.
        @tparam Args Deduced argument type.
        @param arg Argument passed to visitor.
        @return the result of the visitor.
    */
    template <typename Args>
    constexpr auto operator()(Args&& arg) const
    {
        using type = typename type_list::at<List, index>::type;
        return Visitor<type, Args>()(arg);
    }
};

/**
    Helper template to apply a given visitor to every element of the string partition.
    @tparam List The string parition.
    @tparam Visitor Visitor template.
    @tparam Part Type of the visited partition element.
    @tparam Args Argument passed to the visitor (partitioned string).
    @tparam index Index in the partition to apply the visitor at. Used for stop condition in recursion.
*/
template <typename List, template<typename Part, typename Args> class Visitor, size_t index>
class visit_all;

/**
    Helper template to apply a given visitor to every element of the string partition.
    Specialization for the index 0, which applies the visitor to the first element in the list.
    @tparam List The string parition.
    @tparam Visitor Visitor template.
    @tparam Part Type of the visited partition element.
    @tparam Args Argument passed to the visitor (partitioned string).
*/
template <typename List, template<typename Part, typename Args> class Visitor>
class visit_all<List, Visitor, 0> {
public:
    template <typename Args>
    constexpr auto operator()(Args&& args) const
    {
        return visit_at<List, Visitor, 0>()(args);
    }
};

/**
    Helper template to apply a given visitor to every element of the string partition.
    @tparam List The string parition.
    @tparam Visitor Visitor template.
    @tparam Part Type of the visited partition element.
    @tparam Args Argument passed to the visitor (partitioned string).
    @tparam index Index in the partition to start applying the visitor.
*/
template <typename List, template<typename Part, typename Args> class Visitor, size_t index>
class visit_all {
public:
    /**
        Apply the visitor.
        @tparam Args Deduced argument type.
        @param args Argument passed to visitor, the partitioned string.
    */
    template <typename Args>
    constexpr auto operator()(Args&& args) const
    {
        return visit_all<List, Visitor, index - 1>()(args), visit_at<List, Visitor, index>()(args);
    }
};
}

/**
    Helper class to iterate the string partition.
    @tparam List A string partition.
    @tparam Visitor Visitor template. New instance of the Visitor is created for every parition element.
    @tparam Part Type of the visited partition element.
    @tparam Args Argument passed to the Visitor application operator.
*/
template <typename List, template<typename Part, typename Args> class Visitor>
class visit {
public:
    /**
        Iterate over all elements in the string partition.
        @tparam Args Deduced type of the char array.
        @param arg Argument passed to the visitor application operator.
    */
    template <typename Args>
    constexpr auto operator()(Args&& arg) const
    {
        return priv::visit_all<List, Visitor, type_list::count<List>::size - 1>()(arg);
    }
};
}
