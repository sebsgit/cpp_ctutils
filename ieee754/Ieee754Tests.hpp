#pragma once

#include "Ieee754Utils.hpp"

namespace ieee754_test {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

#define STATIC_ASSERT(cond) static_assert(cond, #cond)

    class TestSuite {
        static void testUtils() {
            STATIC_ASSERT (ieee754::priv::numBitsFor(0) == 1);
            STATIC_ASSERT (ieee754::priv::numBitsFor(1) == 1);
            STATIC_ASSERT (ieee754::priv::numBitsFor(2) == 2);
            STATIC_ASSERT (ieee754::priv::numBitsFor(3) == 2);
            STATIC_ASSERT (ieee754::priv::numBitsFor(4) == 3);
        }
        static void testRepresentation() {
            STATIC_ASSERT (ieee754::getSignBit(0.13f) == 0);
            STATIC_ASSERT (ieee754::getExponentBits(0.13f) == 124);
            STATIC_ASSERT (ieee754::getMantissa(0.13f) != 0);
        }
        static void testConversion() {
#define VALIDATE(value) STATIC_ASSERT (ieee754::fromIEEE754(ieee754::toIEEE754(value)) == value)

            VALIDATE(-1231.0123f);
            VALIDATE(8734.99201f);
            VALIDATE(1.0f);

#undef VALIDATE
        }
    };
#undef STATIC_ASSERT
#pragma GCC diagnostic pop
}
