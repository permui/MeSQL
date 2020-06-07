/*
 * MeSQL - base - timer.cppp
 * 
 * This file defines the class Timer declared in timer.hpp .
 * 
 */

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include "timer.hpp"

using namespace std;
using namespace std::chrono;

namespace MeTime {

	// implement class Timer
	Timer::Timer() :fir(),sec(),dur() {}
	void Timer::start() {
		fir = high_resolution_clock::now();
	}
	high_resolution_clock::duration::rep Timer::stop() {
		sec = high_resolution_clock::now();
		dur = sec - fir;
	}
	string Timer::str() const {
		static stringstream ss;
		ss.str("");
		auto d = duration_cast<microseconds>(dur);
		auto l = d.count();
		ss << fixed << setprecision(3);
		if (l < 1000000) ss << l/1000. << " ms";
		else ss << l/1000000. << " s";
		return ss.str();
	}
	string Timer::paren_str() const {
		return "( " + str() + " )";
	}
}