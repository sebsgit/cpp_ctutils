#pragma once

#include <cinttypes>
#include <type_traits>

/// @TODO document and cleanup
namespace sha1_utils {

    namespace priv {
        class sha1_pad {
        public:
            static constexpr unsigned char data[256] = {
                0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            };
        };

        class sha1_length_buffer {
        public:
            constexpr sha1_length_buffer(const uint64_t buffSize)
                : lengthBits(buffSize * 8)
                , data{ (unsigned char)(lengthBits >> 56), (unsigned char)(lengthBits >> 48),
                        (unsigned char)(lengthBits >> 40), (unsigned char)(lengthBits >> 32),
                        (unsigned char)(lengthBits >> 24), (unsigned char)(lengthBits >> 16),
                        (unsigned char)(lengthBits >> 8), (unsigned char)(lengthBits >> 0) }
            {

            }
            const int64_t lengthBits = 0;
            const unsigned char data[8] = {0};
        };
    }

    class sha1_context {
    private:
        /**
            Resets the internal message buffer.
        */
        template <size_t ... index64>
        constexpr sha1_context(const sha1_context& other, std::index_sequence<index64...>)
            :buffer{ 0 }
            ,result{ other.result[0], other.result[1], other.result[2], other.result[3], other.result[4] }
            ,message_length(other.message_length)
            ,buffer_length(0)
        {

        }

        template <size_t N, typename Char>
        constexpr static unsigned char get_item_helper_(const sha1_context& other,
                                           const Char (&message)[N],
                                           const size_t array_offset,
                                           const size_t copy_size,
                                           const size_t i)
        {
            return i < other.buffer_length ? other.buffer[i] : (i - other.buffer_length < copy_size ? message[i - other.buffer_length + array_offset] : '\0');
        }
        /**
            Updates the result buffer with given values.
        */
        template <size_t ... index64>
        constexpr sha1_context(const sha1_context& other, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, std::index_sequence<index64...>)
            :buffer{ other.buffer[index64] ... }
            ,result{ other.result[0] + a, other.result[1] + b, other.result[2] + c, other.result[3] + d, other.result[4] + e }
            ,message_length(other.message_length)
            ,buffer_length(other.buffer_length)
        {

        }
        /**
            Appends the 8 byte length buffer to this context.
        */
        template <size_t ... index56>
        constexpr sha1_context(const sha1_context& other, const priv::sha1_length_buffer& lengthBuffer, std::index_sequence<index56 ...>)
            :buffer{other.buffer[index56] ..., lengthBuffer.data[0], lengthBuffer.data[1], lengthBuffer.data[2],
                lengthBuffer.data[3], lengthBuffer.data[4], lengthBuffer.data[5], lengthBuffer.data[6], lengthBuffer.data[7] }
            ,result{other.result[0], other.result[1], other.result[2], other.result[3], other.result[4]}
            ,message_length(other.message_length + 8)
            ,buffer_length(other.buffer_length + 8)
        {

        }
        /**
            Creates new context by appending some data to the existing other context.
        */
        template <size_t N, size_t ... index, typename Char>
        constexpr sha1_context(const sha1_context& other,
                               const Char (&message)[N],
                               const size_t array_offset,
                               const size_t copy_size,
                               std::index_sequence<index...>)
            :buffer{ get_item_helper_(other, message, array_offset, copy_size, index) ... }
            ,result{other.result[0], other.result[1], other.result[2], other.result[3], other.result[4]}
            ,message_length(other.message_length + copy_size)
            ,buffer_length(other.buffer_length + copy_size)
        {
        }
        static constexpr auto pad_byte(const uint64_t lengthBits)
        {
            return lengthBits < 448 ? (448 - lengthBits) / 8 : (lengthBits > 448 ? (448 + (512 - lengthBits)) / 8 : 64);
        }
    public:
        static constexpr size_t Size = 64;

        constexpr sha1_context() {}

        constexpr auto reset_buffer() const
        {
            return sha1_context(*this, std::make_index_sequence<Size>());
        }

        constexpr auto update_result(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) const
        {
            return sha1_context(*this, a, b, c, d, e, std::make_index_sequence<Size>());
        }

        constexpr auto append_length_buffer(const priv::sha1_length_buffer& lengthBuffer) const
        {
            return sha1_context(*this, lengthBuffer, std::make_index_sequence<56>());
        }

        template <size_t N, typename Char>
        constexpr auto append(const Char (&message)[N], const size_t array_offset, const size_t copy_size) const
        {
            static_assert(std::is_same<char, Char>::value || std::is_same<unsigned char, Char>::value, "cannot append non char data to context.");
            return copy_size == 0 ? *this : sha1_context(*this, message, array_offset, copy_size, std::make_index_sequence<Size>());
        }

        constexpr auto padding_byte() const
        {
            return pad_byte( (this->message_length * 8) % 512 );
        }

        constexpr auto append_padding(const uint64_t padLength, const uint64_t padBufferOffset = 0) const
        {
            return this->append(priv::sha1_pad::data, padBufferOffset, padLength);
        }

        constexpr size_t free_space() const
        {
            return Size - this->buffer_length;
        }

        const unsigned char buffer[Size] = {0};
        const uint32_t result[5] = {0x67452301,
                                 0xEFCDAB89,
                                 0x98BADCFE,
                                 0x10325476,
                                 0xC3D2E1F0};
        const uint64_t message_length = 0;
        const uint64_t buffer_length = 0;
    };

    class sha1_result {
    public:
        constexpr sha1_result(const sha1_context& context)
            :data{ (unsigned char)(context.result[0] >> 24), (unsigned char)(context.result[0] >> 16), (unsigned char)(context.result[0] >> 8), (unsigned char)(context.result[0] >> 0),
                   (unsigned char)(context.result[1] >> 24), (unsigned char)(context.result[1] >> 16), (unsigned char)(context.result[1] >> 8), (unsigned char)(context.result[1] >> 0),
                   (unsigned char)(context.result[2] >> 24), (unsigned char)(context.result[2] >> 16), (unsigned char)(context.result[2] >> 8), (unsigned char)(context.result[2] >> 0),
                   (unsigned char)(context.result[3] >> 24), (unsigned char)(context.result[3] >> 16), (unsigned char)(context.result[3] >> 8), (unsigned char)(context.result[3] >> 0),
                   (unsigned char)(context.result[4] >> 24), (unsigned char)(context.result[4] >> 16), (unsigned char)(context.result[4] >> 8), (unsigned char)(context.result[4] >> 0),
                }
        {}
        constexpr unsigned char operator[](const size_t i) const { return data[i]; }
    private:
        const unsigned char data[20] = {0};
    };

    namespace priv {
        class sha1_compute {
        public:

            constexpr sha1_compute() {}

            template <typename Ctx>
            constexpr uint32_t get_x(const Ctx& context, size_t i) const
            {
                return (((uint32_t)context.buffer[i * 4] << 24) | ((uint32_t)context.buffer[i * 4 + 1] << 16) | ((uint32_t)context.buffer[i * 4 + 2] << 8) | ((uint32_t)context.buffer[i * 4 + 3]));
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
            return source.update_result(update.a, update.b, update.c, update.d, update.e);
        }

        constexpr sha1_context sha1_calc(const sha1_context& source)
        {
            return sha1_update(source, perform_loop<perform_loop_base::sha1_rounds>(source, sha1_compute().add_context_round(source).add_rotate_round()).calculate()._ctx_update);
        }

        template <size_t N>
        constexpr sha1_context sha1_add_data_helper(const sha1_context& context, const char (&array)[N], const size_t array_offset, const size_t copy_size)
        {
            return (context.buffer_length + copy_size) < sha1_context::Size ?
                    context.append(array, array_offset, copy_size)
                  : sha1_add_data_helper( sha1_calc(context.append(array, array_offset, std::min(context.free_space(), copy_size) )).reset_buffer(),
                                     array,
                                     array_offset + std::min(context.free_space(), copy_size),
                                     copy_size - std::min(context.free_space(), copy_size));
        }

        template <size_t N> constexpr sha1_context sha1_add_data(const sha1_context& context, const char (&array)[N]);

        constexpr sha1_context sha1_finalize_unpadded(const sha1_context& context)
        {
            return sha1_calc(context.append_padding(context.free_space()))
                    .reset_buffer()
                    .append_padding(context.padding_byte() - context.free_space(), context.free_space())
                    .append_length_buffer(context.message_length);
        }
    } // ~priv

    template <size_t N>
    constexpr auto sha1_create_context(const char (&message)[N])
    {
        return sha1_add_data(sha1_context(), message);
    }

    template <size_t N>
    constexpr sha1_context sha1_add_data(const sha1_context& context, const char (&array)[N])
    {
      return priv::sha1_add_data_helper(context, array, 0, N - 1);
    }

    constexpr sha1_context sha1_finalize(const sha1_context& context)
    {
        return priv::sha1_calc(context.buffer_length + 8 + context.padding_byte() <= sha1_context::Size ?
                    context.append_padding(context.padding_byte()).append_length_buffer(context.message_length)
                  :priv::sha1_finalize_unpadded(context));
    }

    template <size_t N, typename Char>
    constexpr sha1_result sha1(const Char (&array)[N])
    {
        return sha1_result(sha1_finalize(sha1_add_data(sha1_context(), array)));
    }

}

#ifdef __GNUC__
constexpr unsigned char sha1_utils::priv::sha1_pad::data[];
#endif
