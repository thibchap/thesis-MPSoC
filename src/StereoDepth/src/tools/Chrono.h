/*
 * Author : Thibault Chappuis
 * Date : 6 jan. 2018
 */
#pragma once
#include <chrono>

using namespace std::chrono;

class Chrono {
public:
	/* Constructors */
	Chrono(): time_span(0), counter(0) {}
	/* Methods */
	inline void start() {
		t1 = high_resolution_clock::now();
	}

	inline void stop() {
		t2 = high_resolution_clock::now();
		time_span += duration_cast<duration<double>> (t2 - t1);
		counter++;
	}

	/*
	 * Return time-span in seconds
	 */
	inline double getLastTime() const {
		return duration_cast<duration<double>> (t2 - t1).count();
	}

	inline double getTotalTime() const {
		return time_span.count();
	}

	inline double getAvgTime() const {
		return time_span.count() / (double)counter;
	}

	inline void reset() {
		time_span = duration<double>(0.0);
		counter = 0;
	}

private:
	high_resolution_clock::time_point t1;
	high_resolution_clock::time_point t2;

	duration<double> time_span;
	int counter;
};
