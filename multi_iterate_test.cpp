#include "multi_iterate.hpp"

#include <vector>
#include <array>
#include <iostream>
#include <numeric>

//TODO move this to a separate file
#define TEST_START std::cout << __FILE__ << ", " << __PRETTY_FUNCTION__ << " - test start\n"
#define TEST_END std::cout << __FILE__ << ", " << __PRETTY_FUNCTION__ << " - test end\n"
#define REQ(what) if( !(what) ) { std::cout << __FILE__ << " " << __LINE__ << " !!! => " << #what << '\n'; exit(1); }
#define REQ_THROW(what) do { bool was_exc = false; try { what; } catch(...) { was_exc = true; } if(!was_exc) { std::cout << "!!! expected exception: " << #what << '\n'; exit(1); } } while(0);

static void basicTest() {
    TEST_START;
    std::vector<int> vec{4, 5, 6};
    std::array<float, 3> arr{1, 2, 3};
    std::vector<float> result;
    for (auto it : multi_iter::iterate(arr, vec)) {
        result.push_back(std::get<0>(it) + std::get<1>(it));
    }
    REQ(result.size() == 3);
    REQ(result[0] == 5.0f);
    REQ(result[1] == 7.0f);
    REQ(result[2] == 9.0f);
    for (auto [a, b, c] : multi_iter::iterate(arr, vec, result)) {
        a = 0;
        b = 0;
        c = 0;
    }
    REQ(std::accumulate(result.begin(), result.end(), 0) == 0.0f);
    REQ(std::accumulate(vec.begin(), vec.end(), 0) == 0);
    REQ(std::accumulate(arr.begin(), arr.end(), 0) == 0.0f);
    TEST_END;
}

static void negativeTest() {
    TEST_START;
    std::array<int, 4> arr1;
    std::array<float, 7> arr2;
    REQ_THROW( multi_iter::iterate(arr1, arr2) );
    TEST_END;
}

/**
TODO:
    - test const containers
    - test non-copyable classes

*/

void testMultiIterate() {
    negativeTest();
    basicTest();
}

#ifdef WITH_MAIN
int main() {
	testMultiIterate();
	return 0;
}
#endif
