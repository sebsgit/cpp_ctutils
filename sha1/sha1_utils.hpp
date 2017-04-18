#pragma once

#include <cinttypes>

/// @TODO document and cleanup
namespace sha1_utils {

    class sha1_context {
    public:
        template <size_t N, size_t ... index>
        constexpr sha1_context(const char (&message)[N], std::index_sequence<index...>)
            :w{ message[index] ... }
            ,buff_len(N - 1)
        {

        }

        /**
            Creates new context by appending some data to the existing other context.
        */
        template <size_t N, size_t ... index>
        constexpr sha1_context(const sha1_context& other,
                               const char (&message)[N],
                               const size_t array_offset,
                               const size_t copy_size,
                               std::index_sequence<index...>)
            :w{ get_item_helper_(other, message, array_offset, copy_size, index) ... }
            ,buff_len(other.buff_len + copy_size)
        {
        }

        template <size_t N>
        constexpr static char get_item_helper_(const sha1_context& other,
                                           const char (&message)[N],
                                           const size_t array_offset,
                                           const size_t copy_size,
                                           const size_t i)
        {
            return i < other.buff_len ? other.w[i] : (i - other.buff_len < copy_size ? message[i - other.buff_len + array_offset] : '\0');
        }

        template <size_t ... index64>
        constexpr sha1_context(const sha1_context& other, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, std::index_sequence<index64...>)
            :w{ other.w[index64] ... }
            ,result{ other.result[0] + a, other.result[1] + b, other.result[2] + c, other.result[3] + d, other.result[4] + e }
            ,data_len(other.data_len)
            ,buff_len(other.buff_len)
        {

        }

        const char w[64] = {0};
        const uint32_t result[5] = {0x67452301,
                                 0xEFCDAB89,
                                 0x98BADCFE,
                                 0x10325476,
                                 0xC3D2E1F0};
        const uint64_t data_len = 0;
        const uint64_t buff_len = 0;
    };

    template <size_t N>
    constexpr auto sha1_create_context(const char (&message)[N])
    {
        ///@TODO allow longer messages
        static_assert(N < 16, "can't create context with message longer than 16 chars");
        return sha1_context(message, std::make_index_sequence<N>());
    }


    class sha1_compute {
    public:

        constexpr sha1_compute() {}

        template <typename Ctx>
        constexpr uint32_t get_x(const Ctx& context, size_t i) const
        {
            return (((uint32_t)context.w[i * 4] << 24) | ((uint32_t)context.w[i * 4 + 1] << 16) | ((uint32_t)context.w[i * 4 + 2] << 8) | ((uint32_t)context.w[i * 4 + 3]));
        }

        template <typename Ctx, size_t ... index_16>
        constexpr sha1_compute(const Ctx& context, std::index_sequence<index_16...>)
            : x{ get_x(context, index_16) ... }
        {
        }

        template <size_t ... index_16, size_t ... index_64>
        constexpr sha1_compute(const sha1_compute& source, std::index_sequence<index_16...>, std::index_sequence<index_64...>)
           :x{ source.x[index_16] ..., rotate_left_32(x[index_64 + 16 -3] ^ x[index_64 + 16 -8] ^ x[index_64 + 16 -14] ^ x[index_64 + 16 -16], 1) ... }
        {

        }

        template <typename Ctx>
        constexpr sha1_compute add_context_round(const Ctx& context) const
        {
            return sha1_compute(context, std::make_index_sequence<16>());
        }

        constexpr sha1_compute add_rotate_round() const
        {
            return sha1_compute(*this, std::make_index_sequence<16>(), std::make_index_sequence<64>());
        }

        template <typename T>
        static constexpr T rotate_left_32(T x, int n)
        {
            static_assert(sizeof(x) == 4, "rotate_left_32 must be called with 32-bit value");
            return ((x << n) & 0xFFFFFFFF) | (x >> (32 - n));
        }

        const uint32_t x[80] = {0};

    };

    template <int index>
    class compute_loop_helper;

    template <>
    class compute_loop_helper<0> {
    public:
        static constexpr auto get_f(uint32_t b, uint32_t c, uint32_t d) { return (b & c) | ((~b) & d); }
        static constexpr uint32_t k = 0x5A827999;
    };
    template <>
    class compute_loop_helper<1> {
    public:
        static constexpr auto get_f(uint32_t b, uint32_t c, uint32_t d) { return  b ^ c ^ d; }
        static constexpr uint32_t k = 0x6ED9EBA1;
    };
    template <>
    class compute_loop_helper<2> {
    public:
        static constexpr auto get_f(uint32_t b, uint32_t c, uint32_t d) { return (b & c) | (b & d) | (c & d); }
        static constexpr uint32_t k = 0x8F1BBCDC;
    };
    template <>
    class compute_loop_helper<3> {
    public:
        static constexpr auto get_f(uint32_t b, uint32_t c, uint32_t d) { return b ^ c ^ d; }
        static constexpr uint32_t k = 0xCA62C1D6;
    };
    template <int j>
    class compute_loop_helper {
    public:
        static constexpr auto get_f(uint32_t, uint32_t, uint32_t) { return 0; }
        static constexpr uint32_t k = 0;
    };

    class context_update_helper {
    public:
        constexpr context_update_helper() {}

        template <class Ctx>
        constexpr context_update_helper(const Ctx& ctx)
            :a(ctx.result[0])
            ,b(ctx.result[1])
            ,c(ctx.result[2])
            ,d(ctx.result[3])
            ,e(ctx.result[4])
        {

        }
        constexpr context_update_helper(uint32_t _a, uint32_t _b, uint32_t _c, uint32_t _d, uint32_t _e)
            :a(_a)
            ,b(_b)
            ,c(_c)
            ,d(_d)
            ,e(_e)
        {

        }
        const uint32_t a = 0;
        const uint32_t b = 0;
        const uint32_t c = 0;
        const uint32_t d = 0;
        const uint32_t e = 0;
    };

    class perform_loop_base {
    public:
        static constexpr size_t sha1_rounds = 80;

        constexpr perform_loop_base(const context_update_helper& ctx, const sha1_compute& compCtx)
            :_ctx_update(ctx)
            ,_ctx_compute(compCtx)
        {

        }

        const context_update_helper _ctx_update;
        const sha1_compute _ctx_compute;
    };

    template <size_t j>
    class perform_loop;

    template <>
    class perform_loop<0> : public perform_loop_base {
    public:
        constexpr perform_loop(const context_update_helper& ctx, const sha1_compute& compCtx)
            :perform_loop_base(ctx, compCtx)
        {

        }
        constexpr perform_loop_base calculate() const { return *this; }
    };

    template <size_t j>
    class perform_loop : public perform_loop_base {
        using loop_helper = compute_loop_helper<(sha1_rounds - j) / 20>;
    public:
        template <typename Sha1Ctx>
        constexpr perform_loop(const Sha1Ctx& ctx, const sha1_compute& compCtx)
            :perform_loop_base(context_update_helper(ctx), compCtx)
        {

        }
        constexpr perform_loop(const context_update_helper& ctx, const sha1_compute& compCtx)
            :perform_loop_base(ctx, compCtx)
        {

        }

        constexpr perform_loop_base calculate() const
        {
            return perform_loop<j - 1>(context_update_helper(
                                         sha1_compute::rotate_left_32(this->_ctx_update.a, 5) + loop_helper::get_f(this->_ctx_update.b, this->_ctx_update.c, this->_ctx_update.d) + this->_ctx_update.e + loop_helper::k + this->_ctx_compute.x[sha1_rounds - j],
                                         this->_ctx_update.a,
                                         sha1_compute::rotate_left_32(this->_ctx_update.b, 30),
                                         this->_ctx_update.c,
                                         this->_ctx_update.d),
                                       this->_ctx_compute).calculate();
        }
    };

    constexpr sha1_context sha1_update(const sha1_context& source, const context_update_helper& update)
    {
        return sha1_context(source, update.a, update.b, update.c, update.d, update.e, std::make_index_sequence<64>());
    }

    constexpr sha1_context sha1_calc(const sha1_context& source)
    {
        return sha1_update(source, sha1_utils::perform_loop<perform_loop_base::sha1_rounds>(source, sha1_utils::sha1_compute().add_context_round(source).add_rotate_round()).calculate()._ctx_update);
    }

    template <size_t N, size_t ... index>
    constexpr sha1_context sha1_test_append_some_helper(const sha1_context& context, const char (&array)[N], const size_t array_offset, const size_t copy_size, std::index_sequence<index...>)
    {
        // enumerate the whole context buffer - 16 elements
        return sha1_context(context, array, array_offset, copy_size, std::make_index_sequence<16>());
    }

    /**
        Appends some data to the existing context.
    */
    template <size_t N>
    constexpr sha1_context sha1_test_append_some(const sha1_context& context, const char (&array)[N], const size_t array_offset, const size_t copy_size)
    {
        return sha1_test_append_some_helper(context, array, array_offset, copy_size, std::make_index_sequence<N>());
    }



}
