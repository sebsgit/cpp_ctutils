#pragma once

#include <cstdint>

/**
TODO:
    - document
    - handle special cases (+-Inf, NaN, +-0.0f, ...)
    - simplify and reduce code duplication
*/

#if __cplusplus < 201402L
#error ieee754 requires at least C++14 language standard
#endif

/**
    Utilities to generate a IEEE 754 representation of a float number at compile time.

    -- How to test --
    Include the header "Ieee754Tests.hpp" and let the compiler process it.
*/
namespace ieee754 {
    namespace priv {
        constexpr int numBitsFor(float value) noexcept
        {
            if (value < 0.0f)
                value = -value;
            int result = 1;
            int value_i = static_cast<int>(value);
            while (value_i / 2 > 0) {
                ++result;
                value_i /= 2;
            }
            return result;
        }
    } // namespace priv

    constexpr uint8_t getSignBit(float value) noexcept
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
        if (value == -0.0f)
            return 1 << 7;
        return value >= 0.0f ? 0 : (1 << 7);
#pragma GCC diagnostic pop
    }

    constexpr uint8_t getExponentBits(float value) noexcept {
        if (value == 0.0f)
            return 0;
        if (value < 0.0f)
            value = -value;
        if (value < 1.0f) {
            uint8_t exponent = 1;
            while (value * 2 < 1.0f) {
                exponent++;
                value *= 2;
            }
            return 127 - exponent;
        } else {
            uint8_t exponent = 0;
            int v = static_cast<int>(value);
            while (v) {
                v /= 2;
                if (v)
                    ++exponent;
            }
            return exponent + 127;
        }
    }

    constexpr uint32_t getMantissa(float value) noexcept
    {
        if (value == 0.0f)
            return 0;
        if (value < 0.0f)
            value = -value;
        int bit = 0;
        uint32_t result = 0;
        if (value < 1.0f) {
            while (value < 1.0f)
                value *= 2;
            value -= 1.0f;
            while (bit < 23) {
                value *= 2;
                if (value >= 1.0f) {
                    value -= 1.0f;
                    result |= (1u << (32 - bit - 1));
                }
                ++bit;
            }
            return result;
        } else {
            const int numBits = priv::numBitsFor(value);
            bit = numBits - 1;
            int v = static_cast<int>(value);
            float fract = value - v;
            int i = 1;
            while (i < bit) {
                if (v % 2)
                    result |= (1u << (32 - (bit - i) - 1));
                ++i;
                v /= 2;
            }
            if (fract > 0.0f) {
                while (bit < 23) {
                    fract *= 2;
                    if (fract >= 1.0f) {
                        fract -= 1.0f;
                        result |= (1u << (32 - bit - 1));
                    }
                    ++bit;
                }
            }

            return result;
        }
    }

    constexpr uint32_t toIEEE754(float value) noexcept
    {
        const uint8_t sign = ieee754::getSignBit(value);
        const uint8_t expn = ieee754::getExponentBits(value);
        const uint32_t mant = ieee754::getMantissa(value);
        return (static_cast<uint32_t>(sign) << 24) | (static_cast<uint32_t>(expn) << 23) | (mant >> 9);
    }

    constexpr float fromIEEE754(uint32_t v) noexcept
    {
        // [sign:1] [exponent:8] [mantissa:23]
#define READ_BIT(value, which) ((value) & (1u << (which)))
        if (v == 0)
            return 0.0f;
        int exp_value = 0;
        int currentPow = 1;
        for (int i=8 ;i>0 ; --i) {
            bool val = READ_BIT(v, 32 - i - 1);
            exp_value += (val ? currentPow : 0);
            currentPow *= 2;
        }
        exp_value = exp_value - 127;
        float expo = 1.0f;
        if (exp_value > 0) {
            while (exp_value > 0) {
                expo *= 2;
                --exp_value;
            }
        } else if (exp_value < 0){
            exp_value = -exp_value;
            while (exp_value > 0) {
                expo /= 2;
                --exp_value;
            }
        }
        float mantissaResult = 0.0f;
        int currentDiv = 2;
        for (int i=9 ;i<32 ; ++i) {
            const bool val = READ_BIT(v, 32 - i - 1);
            mantissaResult += (val ? (1.0f / currentDiv) : 0.0f);
            currentDiv *= 2;
        }
        const auto sign = ((READ_BIT(v, 31)) ? -1.0f : 1.0f);
        return sign * expo * (1.0f + mantissaResult);
    }
#undef READ_BIT
} // namespace ieee754;
