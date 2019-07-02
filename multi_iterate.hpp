#pragma once

#include <tuple>
#include <functional>
#include <exception>

namespace multi_iter {
#ifdef __cpp_concepts
template <typename T>
concept bool ForwardIterator = requires(T it) {
    {*it};
    {++it};
    {it == it}
};

template <typename T>
concept bool ForwardIterable = ForwardIterator<decltype (std::begin(std::declval<T>()))>
        && ForwardIterator<decltype (std::end(std::declval<T>()))>;

#define IS_ITERABLE(T) ForwardIterable T

#else
    //TODO do some enable_if
#define IS_ITERABLE(T) typename T
#endif

template <typename ... Args>
class multi_iterator {
	template <IS_ITERABLE(Container)>
	using start_iterator = decltype (std::begin(std::declval<Container>()));

	template <IS_ITERABLE(Container)>
	using end_iterator = decltype (std::end(std::declval<Container>()));

	template <IS_ITERABLE(Container)>
	using iterator_pair = std::pair<start_iterator<Container>, end_iterator<Container>>;

public:
	using iterators = std::tuple<iterator_pair<Args>...>;

    template <typename T>
    explicit multi_iterator(T&& t)
        :_data{std::forward<T>(t)}
    {
    }
    bool is_done() const noexcept {
        return std::get<0>(this->_data).first == std::get<0>(this->_data).second;
    }

    auto& operator++() {
        this->increment(std::make_index_sequence<sizeof...(Args)>());
        return *this;
    }
    auto operator*() {
        return this->deref(std::make_index_sequence<sizeof...(Args)>());
    }

    friend bool operator != (const multi_iterator& pthis, const multi_iterator<std::nullptr_t>& /*sentinel*/) noexcept {
        return !pthis.is_done();
    }
private:
    template <size_t ... index>
    void increment(std::index_sequence<index...>) {
        std::initializer_list<int>{(++std::get<index>(this->_data).first, 0) ...};
    }

    template <size_t ... index>
    auto deref(std::index_sequence<index...>) {
        return std::make_tuple(std::ref(*std::get<index>(this->_data).first) ...);
    }
    template <size_t ... index>
    auto deref(std::index_sequence<index...>) const {
        return std::make_tuple(std::cref(*std::get<index>(this->_data).first) ...);
    }

private:
    iterators _data;
};

template <>
class multi_iterator<std::nullptr_t> {
};

template <typename ... Args>
class multi_adapter {
public:
    template <typename ... T>
    explicit multi_adapter(T&& ... t)
        :_data{{std::begin(t), std::end(t)} ...}
    {
    }
    auto begin() {
        return multi_iterator<Args...>(this->_data);
    }
    auto end() {
        return multi_iterator<std::nullptr_t>();
    }
private:
    typename multi_iterator<Args...>::iterators _data;
};

template <IS_ITERABLE(C), typename ... Args>
auto get_size(C&& c, Args&& ...) {
    return std::size(c);
}

/**
    TODO:
        - allow different container sizes with user-defined fallback values
*/
template <typename ... Args>
auto iterate(Args && ... args) {
    const auto size = get_size(std::forward<Args>(args) ...);
    if (((get_size(args) != size) || ...)) {
        throw std::runtime_error("cannot multi-iterate containers of different sizes");
    }
    return multi_adapter<Args...>(std::forward<Args>(args)...);
}

} // namespace

