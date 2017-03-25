#pragma once

#include "string_part.hpp"
#include "type_list.hpp"

/**
    Provides helper functions to create a compile time partition of a given character sequence.
*/
namespace string_partition {

namespace priv {

/**
    A helper class to choose between "filled" and "regular" @ref string_part, depending on the start index
    in the char array (we want to fill the last part of the partitioned string array to match the size of
    all the other parts).
    @tparam part_size Size of each part in partition.
    @tparam array_size Total size of the partitioned array.
    @tparam start_index Start index in the array.
*/
template <size_t part_size, size_t array_size, size_t start_index>
class declare_part
{
private:
    using data_type = const char[array_size];
    /**
        Helper method to declare return type.
        @return A string part with the size equal to @ref part_size, starting at the @ref start_index
        @note This method is never called at runtime.
    */
    constexpr static auto helper(const char (&data)[array_size])
    {
        return string_part::make_part<part_size, start_index>(data);
    }
    /**
        Helper method to declare return type.
        @return A filled string part with the size equal to @ref part_size, starting at the @ref start_index
        @note This method is never called at runtime.
    */
    constexpr static auto helper_filled(const char (&data)[array_size])
    {
        return string_part::make_filed_part<part_size, start_index, array_size - start_index>(data);
    }
    /// @return true if this part is too small and needs to be filled to match the @ref part_size, false otherwise.
    constexpr static bool is_filled()
    {
        return start_index + part_size > array_size;
    }

public:
    /// final type of this string partition element
    using type = typename std::conditional<is_filled(), decltype(helper_filled(data_type{})), decltype(helper(data_type{}))>::type;
};

/**
    Helper class to recursively partition a given string at compile time.
    @tparam List Type list to keep the part types.
    @tparam part_size size of each string partition.
    @tparam array_size Total size of the string.
    @tparam total_offset Accumulator used to keep correct offset in the char array for each partition element.
    @tparam count A counter to end the recursion.
*/
template <typename List, size_t part_size, size_t array_size, size_t total_offset, size_t count>
class declare_parts;

/**
    Stop condition for recursive paritioning.
    @tparam List Type list to keep the part types.
    @tparam part_size size of each string partition.
    @tparam total_offset Accumulator used to keep correct offset in the char array for each partition element.
    @tparam array_size Total size of the string.
*/
template <typename List, size_t part_size, size_t array_size, size_t total_offset>
class declare_parts<List, part_size, array_size, total_offset, 1>
{
public:
    using type = List;
};

/**
    Helper class to partition a given string at compile time. Use recursion to append array partition types into a type list.
    @tparam List Type list to keep the part types.
    @tparam part_size size of each string partition.
    @tparam array_size Total size of the string.
    @tparam total_offset Accumulator used to keep correct offset in the char array for each partition element.
    @tparam count A counter to end the recursion.
*/
template <typename List, size_t part_size, size_t array_size, size_t total_offset, size_t count>
class declare_parts
{
private:
    using partition_element = typename declare_part<part_size, array_size, total_offset>::type;
    using current_partition = typename type_list::append<List, partition_element>::type;
public:
    using type = typename declare_parts<current_partition, part_size, array_size, total_offset + part_size, count - 1>::type;
};
}

/**
    Helper method to partition a given char array into parts of equal lengths.
    This method is meant to be used at compile time - it returns a type list which can be
    used with a @ref partition_iterator.
*/
template <size_t part_size, size_t array_size_deduced>
constexpr auto make_partition(const char (&data)[array_size_deduced])
{
    /// todo support smaller data sets (part_size > array_size_deduced)
    constexpr auto array_size = array_size_deduced - 1;
    constexpr auto offset = part_size;
    constexpr auto part_count = (array_size / part_size) + ((array_size % part_size) ? 1 : 0);
    using first_part = decltype(string_part::make_part<part_size, 0>(data));
    using partition_start = typename type_list::create<first_part>::type;
    using final_partition = typename priv::declare_parts<partition_start, part_size, array_size, offset, part_count>::type;
    return final_partition();
}

}
