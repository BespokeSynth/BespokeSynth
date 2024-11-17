#include "TempoFollower.h"

TempoFollower::TempoFollower() : 
    previousA(false),
    lastPulseTime(0.0),
    measuringInterval(false)
{
}

// base on tempo_v3.py by @dzintars


double TempoFollower::update(bool currentA, double currentTime, double currentTempo) {
    double newTempo = -1.0;

    // Check transition from 0 to 1
    if (!previousA && currentA) {
        double beatsPerSecond = currentTempo / 60.0;
        
        if (measuringInterval) {
            double timeDifferenceBeats = currentTime - lastPulseTime;
            double timeDifferenceSeconds = timeDifferenceBeats / beatsPerSecond;

            if (timeDifferenceSeconds >= minValidInterval && 
                timeDifferenceSeconds <= maxValidInterval) {
                
                intervals.push_back(timeDifferenceSeconds);
                if (intervals.size() > maxIntervals) {
                    intervals.pop_front();
                }

                double averageInterval = 0.0;
                for (double interval : intervals) {
                    averageInterval += interval;
                }
                averageInterval /= intervals.size();

                newTempo = 60.0 / averageInterval;
            } 
            else if (timeDifferenceSeconds > maxValidInterval) {
                measuringInterval = false;
                intervals.clear();
            }
        }

        lastPulseTime = currentTime;
        measuringInterval = true;
    }

    // Check timeout
    double currentInterval = (currentTime - lastPulseTime) / (currentTempo / 60.0);
    if (measuringInterval && currentInterval > maxValidInterval) {
        measuringInterval = false;
        intervals.clear();
    }

    previousA = currentA;
    return newTempo;
}

void TempoFollower::reset() {
    previousA = false;
    lastPulseTime = 0.0;
    intervals.clear();
    measuringInterval = false;
} 