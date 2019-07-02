#pragma once

#include <iostream>

// macros for very simple unit tests

#define TEST_START std::cout << __FILE__ << ", " << __PRETTY_FUNCTION__ << " - test start\n"
#define TEST_END std::cout << __FILE__ << ", " << __PRETTY_FUNCTION__ << " - test end\n"
#define REQ(what) if( !(what) ) { std::cout << __FILE__ << " " << __LINE__ << " !!! => " << #what << '\n'; exit(1); }
#define REQ_THROW(what) do { bool was_exc = false; try { what; } catch(...) { was_exc = true; } if(!was_exc) { std::cout << "!!! expected exception: " << #what << '\n'; exit(1); } } while(0);

