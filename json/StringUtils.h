#pragma once

#include <cstdint>
#include <string>

namespace ctjson
{
    class StringView
    {
    public:
        constexpr explicit StringView(const char *src, const size_t size) noexcept :
            ptr_ {src},
            size_ {size}
        {}
        template <size_t N>
        constexpr bool equals(const char (&other)[N]) const noexcept
        {
            if (size_ != N - 1) {
                return false;
            }
            for (size_t i = 0; i < N - 1; ++i) {
                if (other[i] != ptr_[i]) {
                    return false;
                }
            }
            return true;
        }
        std::string toString() const
        {
            return std::string(ptr_, size_);
        }
    private:
        const char * ptr_;
        const size_t size_;
    };

	template <const char* string, size_t start, size_t length, bool is_empty = (length == 0)>
	class String;

	using EmptyString = String<nullptr, 0, 0>;

	template <const char* string, size_t start, size_t length>
	class String<string, start, length, true> {
	public:
		using Trimmed = String<string, start, length, true>;
		template <size_t from, size_t to>
		using Substring = Trimmed;

        static constexpr StringView asStringView() noexcept
        {
            return StringView(nullptr, 0);
        }

		template <size_t N>
		static constexpr bool equals(const char(&other)[N]) noexcept
		{
			return N == 1;
		}

		static constexpr char at(size_t idx) noexcept
		{
			return static_cast<char>(0);
		}

		static constexpr size_t size() noexcept
		{
			return 0;
		}

		static constexpr bool empty() noexcept
		{
			return true;
		}

		static constexpr char first() noexcept
		{
			return at(0);
		}

		static constexpr char last() noexcept
		{
			return at(0);
		}
	};

	template <const char* string, size_t start, size_t length>
	class String<string, start, length, false> {
		static constexpr size_t trimmedStart() noexcept
		{
			for (size_t i = start; i < start + length; ++i) {
				if (string[i] != ' ' && string[i] != '\t' && string[i] != '\n') {
					return i;
				}
			}
			return start;
		}

		static constexpr size_t trimmedLength() noexcept
		{
			size_t new_len = length - (trimmedStart() - start);
			while (string[trimmedStart() + new_len - 1] == ' ' || string[trimmedStart() + new_len - 1] == '\t' || string[trimmedStart() + new_len - 1] == '\n') {
				new_len--;
				if (new_len == 1) {
					break;
				}
			}
			return new_len;
		}

		static constexpr size_t trim_pos_beg = trimmedStart();
		static constexpr size_t trim_len = trimmedLength();

	public:
		using Trimmed = String<string, trim_pos_beg, trim_len>;

		template <size_t from, size_t to>
		using Substring = String<string, start + from, to - from>;

        static constexpr StringView asStringView() noexcept
        {
            return StringView(string + start, length);
        }

		static constexpr size_t size() noexcept
		{
			return length;
		}

		static constexpr char at(size_t idx) noexcept
		{
			if (idx >= length) {
				return static_cast<char>(0);
			}
			return string[start + idx];
		}

		static constexpr char first() noexcept
		{
			return at(0);
		}

		static constexpr char last() noexcept
		{
			return at(size() - 1);
		}

		static constexpr bool empty() noexcept
		{
			return size() == 0;
		}

		template <size_t N>
		static constexpr bool equals(const char(&other)[N]) noexcept
		{
			if constexpr (length != N - 1) {
				return false;
			}
			for (size_t i = 0; i < N - 1; ++i) {
				if (other[i] != string[i + start]) {
					return false;
				}
			}
			return true;
		}

		template <size_t N>
		static constexpr size_t find(const char(&other)[N], size_t start_pos = 0) noexcept
		{
			if constexpr (N - 1 > length) {
				return std::string::npos;
			}
			for (size_t i = start + start_pos; i < start + length; ++i) {
				if (i + N - 1 > start + length) {
					return std::string::npos;
				}
				if (string[i] == other[0]) {
					bool match = true;
					for (size_t j = 1; j < N - 1; ++j) {
						if (string[i + j] != other[j]) {
							match = false;
							break;
						}
					}
					if (match) {
						return i - start;
					}
				}
			}
			return std::string::npos;
		}
		static constexpr int32_t toInt() noexcept
		{
			int32_t result = 0;
			for (size_t i = start; i < start + length; ++i) {
				if (string[i] < '0' || string[i] > '9') {
					break;
				}
				result = result * 10 + (string[i] - '0');
			}
			return result;
		}

		static std::string toString() noexcept
		{
			return std::string(string + start, length);
		}
	};

	template <typename StringValue>
	class StringOps {
	public:
		template <size_t idx>
		using left = typename StringValue::template Substring<0, idx>;

		template <size_t idx>
		using right = std::conditional_t<idx != std::string::npos,
			typename StringValue::template Substring<idx + 1, StringValue::size()>,
			typename StringValue::template Substring<0, 0>>;
	};
} // namespace ctjson
