#pragma once

#include <tuple>
#include <functional>

#include <exception>

namespace multi_iter {
template <typename Container>
class adapter {
    using start_iterator = decltype (std::begin(std::declval<Container>()));
    using end_iterator = decltype (std::end(std::declval<Container>()));
public:
    template <typename C>
    explicit adapter(C&& t)
        : _it(std::begin(std::forward<Container>(t)))
        , _end(std::end(std::forward<Container>(t)))
    {
    }
    bool is_done() const noexcept {
        return this->_it == this->_end;
    }
    void advance() {
        ++this->_it;
    }
    decltype(auto) deref() {
        return *this->_it;
    }
private:
    start_iterator _it;
    const end_iterator _end;
};

template <typename ... Args>
class multi_iterator {
public:
    template <typename T>
    explicit multi_iterator(T&& t)
        :_data{std::forward<T>(t)}
    {
    }
    bool is_done() const noexcept {
        return std::get<0>(this->_data).is_done();
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
        std::initializer_list<int>{(std::get<index>(this->_data).advance(), 0) ...};
    }

    template <size_t ... index>
    auto deref(std::index_sequence<index...>) {
        return std::make_tuple(std::ref(std::get<index>(this->_data).deref()) ...);
    }
    template <size_t ... index>
    auto deref(std::index_sequence<index...>) const {
        return std::make_tuple(std::cref(std::get<index>(this->_data).deref()) ...);
    }

private:
    std::tuple<adapter<Args>...> _data;
};

template <>
class multi_iterator<std::nullptr_t> {
public:
    explicit multi_iterator()
    {
    }
};

template <typename ... Args>
class multi_adapter {
public:
    template <typename ... T>
    explicit multi_adapter(T&& ... t)
        :_data{std::forward<T>(t) ...}
    {
    }
    auto begin() {
        return multi_iterator<Args...>(this->_data);
    }
    auto end() {
        return multi_iterator<std::nullptr_t>();
    }
private:
    std::tuple<adapter<Args>...> _data;
};

template <typename C, typename ... Args>
auto get_size(C&& c, Args&& ...) {
    return std::size(c);
}

/**
    TODO:
        - allow different container sizes with user-defined fallback values
        - refactor to not use the adapter
*/
template <typename ... Args>
auto iterate(Args && ... args) {
    const auto size = get_size(std::forward<Args>(args) ...);
    if (((std::size(args) != size) || ...)) {
        throw std::runtime_error("cannot multi-iterate containers of different sizes");
    }
    return multi_adapter<Args...>(std::forward<Args>(args)...);
}

} // namespace

