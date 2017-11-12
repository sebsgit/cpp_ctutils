#!/bin/bash
g++-5.4 -std=c++1z cpp_utils_test.cpp -I. -Wall
if [[ $? -eq 0 ]]; then
	clang++-4.0 cpp_utils_test.cpp -I. -std=c++1z -Wall
	if [[ $? -ne 0 ]]; then
		echo -e "\nclang++-4.0 failed.\n"
	fi
else
	echo -e "\ngcc-5.4 failed.\n"
fi
