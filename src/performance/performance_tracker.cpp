/**
* @file performance_tracker.cpp
 * @brief Performance tracking implementation
 * @author Sam Hoque
 *
 * License: Do whatever the fuck you want
 */

#include "performance_tracker.h"

PerformanceTracker::PerformanceTracker()
    : startTime(), reactionCount(0), totalReaction(0.0) {
    QueryPerformanceFrequency(&frequency);
}

void PerformanceTracker::startCounter() {
    QueryPerformanceCounter(&startTime);
}

void PerformanceTracker::stopCounter() {
    LARGE_INTEGER endTime;
    QueryPerformanceCounter(&endTime);

    const double elapsedTime = (static_cast<double>(endTime.QuadPart) - static_cast<double>(startTime.QuadPart)) * 1000.0 / static_cast<double>(frequency.QuadPart);
    reactionCount++;
    totalReaction += elapsedTime;
}

double PerformanceTracker::getAverageReaction() const {
    if (reactionCount == 0) return 0.0;
    return totalReaction / reactionCount;
}