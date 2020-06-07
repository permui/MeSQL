/*
 * MeSQL - base - timer.hpp
 * 
 * This file declares class Timer for time record.
 * 
 */

#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;

namespace MeTime {
	
	class Timer {
	private:
		high_resolution_clock::time_point fir;
		high_resolution_clock::time_point sec;
		high_resolution_clock::duration dur;
	public:
		Timer();
		void start();
		high_resolution_clock::duration::rep stop();
		string str() const;
		string paren_str() const;
	};

}

#endif