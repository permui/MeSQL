/*
 * MeSQL - Color - color.cpp
 * 
 * This file implements what is declared in color.hpp .
 * 
 */

#include <string>
#include <sstream>
#include "color.hpp"

using namespace std;

string colorful(const string &inp,COLOR color) {
    stringstream ss;
    ss << "\033[1;" << static_cast<int>(color) << "m";
    ss << inp << "\033[0m";
    return ss.str();
}