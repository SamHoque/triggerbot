/**
* @file performance_tracker.h
 * @brief Performance tracking utilities
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#pragma once

#include <windows.h>
#include <iostream>

/**
 * @class PerformanceTracker
 * @brief Tracks and reports reaction times
 */
class PerformanceTracker {
public:
    /**
     * @brief Constructor initializes counters
     */
    PerformanceTracker();

    /**
     * @brief Start timing a reaction
     */
    void startCounter();

    /**
     * @brief Stop timing and report result
     */
    void stopCounter();

    /**
     * @brief Get average reaction time
     * @return Average reaction time in milliseconds
     */
    [[nodiscard]] double getAverageReaction() const;

private:
    LARGE_INTEGER frequency{};
    LARGE_INTEGER startTime;
    int reactionCount;
    double totalReaction;
};