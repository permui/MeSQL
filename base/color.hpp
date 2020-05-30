/*
 * MeSQL - Color - color.hpp
 * 
 * This file declare necessary function and types for color print.
 * 
 */

#ifndef COLOR_HPP
#define COLOR_HPP

#include <string>

using namespace std;

enum class COLOR : int {
    black = 30, red = 31, green = 32, yellow = 33,
    blue = 34, purple = 35, cyan = 36, white = 37
};

string colorful(const string &inp,COLOR color);

#endif