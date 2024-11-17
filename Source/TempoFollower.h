#pragma once

#include <vector>
#include <deque>

class TempoFollower {
private:
    bool previousA;
    double lastPulseTime;
    std::deque<double> intervals;
    static constexpr int maxIntervals = 5;
    static constexpr double minValidInterval = 0.2;  // seconds
    static constexpr double maxValidInterval = 3.0;  // seconds
    bool measuringInterval;

public:
    TempoFollower();

    // Updates tempo based on input signal
    // Returns new tempo if changed, otherwise -1
    double update(bool currentA, double currentTime, double currentTempo);

    void reset();
}; 