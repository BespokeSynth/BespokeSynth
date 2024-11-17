#pragma once

#include <vector>
#include <deque>

class TempoFollower {
private:
    bool previousA = false;
    double lastPulseTime = 0.0;
    std::deque<double> intervals;
    static constexpr int maxIntervals = 5;
    static constexpr double minValidInterval = 0.2;  // seconds
    static constexpr double maxValidInterval = 3.0;  // seconds
    bool measuringInterval = false;

public:
    TempoFollower() = default;

    // Обновляет темп на основе входного сигнала
    // Возвращает новый темп, если он был изменен, иначе -1
    double update(bool currentA, double currentTime, double currentTempo) {
        double newTempo = -1.0;

        // Проверяем переход из 0 в 1
        if (!previousA && currentA) {
            double beatsPerSecond = currentTempo / 60.0;
            
            if (measuringInterval) {
                double timeDifferenceBeats = currentTime - lastPulseTime;
                double timeDifferenceSeconds = timeDifferenceBeats / beatsPerSecond;

                if (timeDifferenceSeconds >= minValidInterval && 
                    timeDifferenceSeconds <= maxValidInterval) {
                    
                    // Добавляем новый интервал
                    intervals.push_back(timeDifferenceSeconds);
                    if (intervals.size() > maxIntervals) {
                        intervals.pop_front();
                    }

                    // Вычисляем средний интервал
                    double averageInterval = 0.0;
                    for (double interval : intervals) {
                        averageInterval += interval;
                    }
                    averageInterval /= intervals.size();

                    // Вычисляем новый темп
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

        // Проверяем тайм-аут
        double currentInterval = (currentTime - lastPulseTime) / (currentTempo / 60.0);
        if (measuringInterval && currentInterval > maxValidInterval) {
            measuringInterval = false;
            intervals.clear();
        }

        previousA = currentA;
        return newTempo;
    }

    // Сброс состояния
    void reset() {
        previousA = false;
        lastPulseTime = 0.0;
        intervals.clear();
        measuringInterval = false;
    }
};
