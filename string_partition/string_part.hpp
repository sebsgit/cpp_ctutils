#pragma once

#include <utility>
#include <cstddef>
#include <string>
#include <array>

/**
    Module provides classes representing single elements in the compile time string partition.
*/
namespace string_part {

/**
    Compile time string part. Initializes the data using characters from a given char array
    with a custom offset.
    @tparam offset Offset in the char array.
    @tparam index Index sequence. Size of this part is equal to the length of the index sequence.
    @note This class is not meant to be created directly, use the @ref make_part helper function.
*/
template <size_t offset, size_t ... index>
class string_part {
    static_assert(sizeof ... (index) > 0 , "can't create null sequence");
    template <size_t ... index_seq>
    constexpr auto to_array_helper(std::index_sequence<index_seq...>) const
    {
        return std::array<uint8_t, sizeof ... (index_seq)>{static_cast<uint8_t>(this->_data[index_seq]) ...};
    }
public:
    /**
        Initializes this string part using a given character array.
        @tparam size Deduced size of the char array.
        @param data Character array to copy the data from.
    */
    template <size_t size>
    constexpr string_part(const char (&data)[size])
        :_data{ data[index + offset]... }
    {
    }
    /**
        Returns the character at given position.
        @param i Index to query the character from.
        @return character at given index.
        @note This function does not perform any bounds checking.
    */
    constexpr char data(size_t i) const { return this->_data[i]; }
    static constexpr size_t size() { return sizeof...(index); }
    /// @return this array as std::string object
    std::string to_string() const
    {
        return std::string({_data[index] ...});
    }
    /// @return this string part as array of bytes
    constexpr std::array<uint8_t, sizeof ... (index)> to_array() const
    {
        return to_array_helper(std::make_index_sequence<sizeof ... (index)>());
    }
private:
    const char _data[sizeof ... (index) + 1];
};

namespace priv {
    // MSVCPP complains when using "const T&" and passing a template parameter
    // as an argument
    template <typename T>
    constexpr T max(T a, T b)
    {
        return a > b ? a : b;
    }
}

/**
    Compile time string part. Initializes the data using characters from a given char array,
    using a custom offset. This part can be bigger than a given array - missing characters are
    filled with '\0' character.
    @tparam offset Custom offset applied when fetching the data.
    @tparam Size Total size of this part, can be longer than the given index sequence.
    @tparam index Index sequence to iterate the char array.
    @note This class is not meant to be created directly, use the @ref make_filled_part helper function.
*/
template <size_t offset, size_t Size, size_t ... index>
class filled_string_part {
    static constexpr auto part_size = priv::max(sizeof ... (index) + 1, Size);

    template <size_t ... index_seq>
    constexpr auto to_array_helper(std::index_sequence<index_seq...>) const
    {
        return std::array<uint8_t, sizeof ... (index_seq)>{static_cast<uint8_t>(this->_data[index_seq]) ...};
    }
public:
    /**
        Initializes the data using a given char array.
        @tparam array_size Deduced length of the parameter.
        @param data Char array to copy the data from.
    */
    template <size_t array_size>
    constexpr filled_string_part(const char (&data)[array_size])
        :_data{ data[index + offset]... }
    {

    }
    /// @return Size of the underlying data.
    static constexpr size_t size() { return part_size; }
    /**
        Returns the character at given position.
        @param i Index to query the character from.
        @return character at given index.
        @note This function does not perform any bounds checking.
    */
    constexpr char data(size_t i) const { return this->_data[i]; }
    /// @return this array as std::string object
    std::string to_string() const
    {
        return std::string({_data[index] ...});
    }
    /// @return this string part as array of bytes
    constexpr std::array<uint8_t, part_size> to_array() const
    {
        return to_array_helper(std::make_index_sequence<part_size>());
    }
private:
    const char _data[part_size] = '\0';
};

namespace priv {
// a helper template used to deduce the index sequence and pass to string_part constructor
template <size_t size, size_t offset, size_t ... index>
constexpr auto make_part_helper(const char (&data)[size], const std::index_sequence<index...>&)
{
    return string_part<offset, index...>(data);
}

// a helper template used to deduce the index sequence and pass to filled_string_part constructor
template <size_t total_size, size_t offset, size_t size, size_t ... index>
constexpr auto make_filled_part_helper(const char (&data)[size], const std::index_sequence<index...>&)
{
    return filled_string_part<offset, total_size, index...>(data);
}
}

/**
    Create a compile-time partition element from a given char array. Element provides a read-only
    compile time view of the string.
    @tparam size Size of the partition element.
    @tparam offset Offset applied when fetching the data from the array.
    @tparam array_size Deduced char array size.
    @return Partition element, starting at given @ref offset, with the length equal to @ref size.
*/
template <size_t size, size_t offset, size_t array_size>
constexpr auto make_part(const char (&data)[array_size])
{
    return priv::make_part_helper<array_size, offset>(data, std::make_index_sequence<size>());
}

/**
    Create a compile-time partition element from a given char array. Element provides a read-only
    compile time view of the string. If the given char array is smaller than requested size,
    the remaining characters are filled with '\0'.
    @tparam total_size Size of the partition element.
    @tparam offset Offset applied when fetching the data from the array.
    @tparam index_length Length of the indexing sequence.
    @tparam array_size Deduced length of the character sequence.
    @return Partition element, starting at given @ref offset, with the length equal to @ref total_size.
*/
template <size_t total_size, size_t offset, size_t index_length, size_t array_size>
constexpr auto make_filed_part(const char (&data)[array_size])
{
    return priv::make_filled_part_helper<total_size, offset>(data, std::make_index_sequence<index_length>());
}

}
