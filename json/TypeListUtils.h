#pragma once

#include <type_traits>

namespace ctjson
{
	template <typename... Args>
	class TypeList;

	class EmptyTypeList {
	public:
		using type = std::false_type;
		using next = std::false_type;

		static constexpr size_t length{ 0 };

		template <typename U>
		using Append = TypeList<U>;

		template <size_t idx>
		using At = std::false_type;
	};

	template <typename T>
	class TypeList<T> {
	public:
		using type = T;
		using next = std::false_type;

		template <typename U>
		using Append = TypeList<T, U>;

		static constexpr size_t length{ 1 };

		template <size_t idx>
		using At = std::conditional_t<idx == 0, T, std::false_type>;
	};

	template <typename T, typename... Args>
	class TypeList<T, Args...> {
	public:
		using type = T;
		using next = TypeList<Args...>;

		template <typename U>
		using Append = TypeList<T, Args..., U>;

		static constexpr size_t length{ 1 + next::length };

		template <size_t idx>
		using At = std::conditional_t<idx == 0, T, typename next::template At<idx - 1>>;
	};

    template <typename ... Args>
    struct IsTypeList
    {
        static constexpr bool value {false};
    };

    template <typename ... Args>
    struct IsTypeList<TypeList<Args ...>>
    {
        static constexpr bool value {true};
    };
} // namespace ctjson
